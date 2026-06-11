# IPC HPS ↔ NIOS por FIFO (REQ-17)

Comunicación inter-procesador para compartir el stream de audio entre el
HPS (ARM, lee el WAV de la SD) y el NIOS II (reproduce el audio).

## Flujo de datos

```
HPS (ARM)                                       NIOS II
lee WAV de la SD card                           lee del FIFO
       |                                            |
       v  escribe muestra (32-bit)                  v  lee muestra (32-bit)
+--------------------------------------------------------------+
|         Avalon FIFO Memory  (profundidad 1024, 32-bit)        |
|         fifo_hps_nios                                          |
+--------------------------------------------------------------+
                                                    |
                                                    v
                              Audio Sample Input (R_DSP) -> AudioFilter -> codec
```

El HPS es el **productor** (escribe muestras), el NIOS es el **consumidor**
(las lee y las pasa a la cadena de audio).

## Configuración del FIFO

- IP: Avalon FIFO Memory Intel FPGA IP (`altera_avalon_fifo`)
- Profundidad: 1024 palabras
- Ancho de datos: 32 bits
- Modo: single clock (50 MHz, dominio del FPGA)
- Backpressure: habilitado
- Status interface (input/CSR): habilitado, visible para HPS y NIOS

## Mapa de direcciones

### Vista NIOS II (direcciones directas)

| Interfaz | Dirección | Acceso | Uso |
|----------|-----------|--------|-----|
| `fifo.out` (datos) | `0x000A_0000` | R | NIOS lee muestras |
| `fifo.in_csr` (status) | `0x000A_0020` | R/W | NIOS consulta nivel/estado |

### Vista HPS / ARM (a través del Lightweight HPS-to-FPGA bridge)

La base del Lightweight bridge en la DE1-SoC (Cyclone V) es `0xFF20_0000`
(valor fijo de hardware). Las direcciones del HPS = base + offset.

| Interfaz | Dirección | Acceso | Uso |
|----------|-----------|--------|-----|
| `fifo.in` (datos) | `0xFF20_0000` | W | HPS escribe muestras |
| `fifo.in_csr` (status) | `0xFF2A_0020` | R/W | HPS consulta espacio/estado |

> NOTA: La dirección del `in_csr` desde el HPS (`0xFF2A_0020`) hay que
> confirmarla al generar el `hps_0.h` con el SoC EDS. El offset proviene del
> Address Map de Platform Designer. La base `0xFF200000` del bridge es estándar
> de la DE1-SoC.

## Registros del CSR (status del FIFO)

El `in_csr` es el control/status slave del FIFO. Sus registros (offsets en
bytes desde la base del csr):

| Offset | Registro | Descripción |
|--------|----------|-------------|
| `0x00` | LEVEL | número de palabras actualmente en el FIFO |
| `0x04` | STATUS | flags de estado (ver máscaras abajo) |
| `0x08` | EVENT | eventos de interrupción (write-1-to-clear) |
| `0x0C` | IENABLE | habilitación de interrupciones |
| `0x10` | ALMOSTFULL | umbral almost-full |
| `0x14` | ALMOSTEMPTY | umbral almost-empty |

### Máscaras del registro STATUS / EVENT

| Máscara | Valor | Significado |
|---------|-------|-------------|
| F | `0x01` | FIFO lleno (full) |
| E | `0x02` | FIFO vacío (empty) |
| AF | `0x04` | almost full |
| AE | `0x08` | almost empty |
| OVF | `0x10` | overflow (se escribió estando lleno) |
| UDF | `0x20` | underflow (se leyó estando vacío) |

## Uso desde el HPS (productor) - código ARM

```c
#include <stdint.h>

#define HPS_LW_BRIDGE_BASE   0xFF200000U
#define FIFO_IN_DATA         (HPS_LW_BRIDGE_BASE + 0x00000)  /* fifo.in    */
#define FIFO_IN_CSR          (HPS_LW_BRIDGE_BASE + 0xA0020)  /* fifo.in_csr*/

/* offsets del CSR */
#define FIFO_LEVEL_REG       0x00
#define FIFO_STATUS_REG      0x04
#define FIFO_STATUS_FULL     0x01

/* NOTA: en Linux sobre el HPS hay que mapear estas direcciones físicas
 * con mmap() sobre /dev/mem. En bare-metal ARM se acceden directamente. */

/* Escribir una muestra al FIFO, esperando si está lleno */
void fifo_write_sample(uint32_t sample) {
    volatile uint32_t *csr  = (volatile uint32_t *)FIFO_IN_CSR;
    volatile uint32_t *data = (volatile uint32_t *)FIFO_IN_DATA;

    /* esperar mientras el FIFO esté lleno */
    while (csr[FIFO_STATUS_REG/4] & FIFO_STATUS_FULL) {
        /* busy-wait, o ceder el CPU en un SO */
    }
    *data = sample;
}
```

## Uso desde el NIOS II (consumidor) - código bare metal

```c
#include "memory_map.h"

/* Leer una muestra del FIFO si hay datos disponibles.
 * Devuelve 1 si leyó una muestra, 0 si el FIFO estaba vacío. */
int fifo_read_sample(uint32_t *sample_out) {
    uint32_t level = REG_READ(FIFO_OUT_CSR_BASE, FIFO_LEVEL_REG);
    if (level == 0) {
        return 0;  /* FIFO vacío, no hay nada que leer */
    }
    *sample_out = REG_READ(FIFO_OUT_BASE, 0x00);
    return 1;
}

/* Ejemplo de loop de reproducción: pasar del FIFO a la cadena de audio */
void audio_playback_loop(void) {
    uint32_t sample;
    while (1) {
        if (fifo_read_sample(&sample)) {
            /* escribir la muestra al Audio Sample Input (R_DSP) */
            uint32_t st = REG_READ(AUDIO_SAMPLE_INPUT_BASE, SAMPLE_STATUS_OFFSET);
            if (!(st & SAMPLE_STATUS_FIFO_FULL)) {
                REG_WRITE(AUDIO_SAMPLE_INPUT_BASE, SAMPLE_WRITE_OFFSET,
                          sample & 0xFFFF);
            }
        }
    }
}
```

## Notas de implementación

1. **Ancho 32-bit, muestras de 16-bit**: el FIFO es de 32 bits pero las
   muestras de audio son 16-bit. Se puede:
   - Enviar una muestra de 16 bits por escritura (desperdiciando los 16 bits
     altos, más simple), o
   - Empaquetar dos muestras de 16 bits por palabra de 32 (más eficiente).
   Coordinar con R_DSP qué formato espera el Audio Sample Input.

2. **Sincronización**: el HPS no debe escribir más rápido de lo que el NIOS
   consume. El FIFO de 1024 palabras da un colchón (~21 ms a 48 kHz), pero el
   HPS debe verificar el flag FULL antes de escribir.

3. **IRQ opcional**: el FIFO tiene IRQ habilitado. El NIOS puede usar la
   interrupción almost-empty para saber cuándo rellenar, en vez de hacer
   polling. (Pendiente de implementar si se requiere.)

4. **Acceso desde Linux (HPS)**: si el HPS corre Linux, las direcciones
   físicas se mapean con `mmap()` sobre `/dev/mem`. Si corre bare-metal, se
   accede directo a las direcciones físicas.
