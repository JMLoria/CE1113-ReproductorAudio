# Mapa de Memoria del Sistema

## Vista desde NIOS II
Direcciones asignadas manualmente para mantener una organización por
categoría.

| Componente | Base | End | Tamaño | Categoría |
|------------|------|-----|--------|-----------|
| RAM (On-Chip) | `0x0000_0000` | `0x0000_FFFF` | 64 KB | Memoria |
| Nios II Debug | `0x0001_0000` | `0x0001_07FF` | 2 KB | CPU |
| System ID | `0x0002_0000` | `0x0002_0007` | 8 B | Identificación |
| JTAG UART | `0x0003_0000` | `0x0003_0007` | 8 B | Comunicación |
| Interval Timer | `0x0004_0000` | `0x0004_001F` | 32 B | Timing |
| Buttons PIO (IRQ) | `0x0005_0000` | `0x0005_000F` | 16 B | Entrada |
| Switches PIO | `0x0005_1000` | `0x0005_100F` | 16 B | Entrada |
| LEDs PIO | `0x0005_2000` | `0x0005_200F` | 16 B | Salida |
| **HEX Display Controller** | **`0x0006_0000`** | **`0x0006_000F`** | **16 B** | **Custom IP (REQ-04)** |
| Audio IP | `0x0007_0000` | `0x0007_000F` | 16 B | Audio (reproducción) |
| Audio Config | `0x0007_2000` | `0x0007_200F` | 16 B | Audio (config I2C) |

## IRQs del NIOS II

| IRQ | Componente |
|-----|------------|
| 1 | JTAG UART |
| 2 | Interval Timer |
| 3 | Buttons PIO |
| 4 | Audio IP |

## Registros del HEX Display Controller
Módulo personalizado diseñado por R_SoC. Controla los 6 displays de
7 segmentos de la DE1-SoC (HEX0–HEX5).

| Offset | Nombre | R/W | Descripción |
|--------|--------|-----|-------------|
| 0x00 | CONTROL | R/W | bit 0: enable, bit 1: modo (0=tiempo MM:SS, 1=hex 24-bit) |
| 0x04 | STATUS | R | bit 0: ready (siempre 1) |
| 0x08 | DATA | R/W | dato a mostrar (interpretación según modo) |
| 0x0C | - | - | Reservado |

### Modos de operación
**Modo 0 (tiempo MM:SS)**:
- `DATA` se interpreta como segundos totales (uint16, máx 99 min)
- HEX3..HEX0 muestran MM:SS, HEX5 y HEX4 apagados
- Ejemplo: `DATA = 125` → muestra "02:05" (HEX3=0, HEX2=2, HEX1=0, HEX0=5)
**Modo 1 (hex 24-bit)**:
- `DATA` se interpreta como número hex de 24 bits
- HEX5..HEX0 muestran los 6 dígitos hex
- Ejemplo: `DATA = 0x123ABC` → HEX5=1, HEX4=2, HEX3=3, HEX2=A, HEX1=B, HEX0=C

### Ejemplo de uso (NIOS II Bare Metal)
```c
#include "memory_map.h"
/* Mostrar 02:05 en los displays */
REG_WRITE(HEX_DISPLAY_BASE, HEX_CONTROL_OFFSET,
          HEX_CTRL_ENABLE | HEX_CTRL_MODE_TIME);
REG_WRITE(HEX_DISPLAY_BASE, HEX_DATA_OFFSET, 125);  /* 125 segundos */
```
## Registros del Audio IP
Audio IP de Intel University Program (altera_up_avalon_audio).
Solo reproducción (Audio Out), 16-bit, 48 kHz. IRQ 4 al NIOS.
| Offset | Nombre | R/W | Descripción |
|--------|--------|-----|-------------|
| 0x00 | CONTROL | R/W | bit 0: RE (read IRQ en), bit 1: WE (write IRQ en), bit 2: CR (clear read FIFO), bit 3: CW (clear write FIFO) |
| 0x04 | FIFOSPACE | R | espacio disponible en FIFOs (ver desglose abajo) |
| 0x08 | LEFTDATA | R/W | dato del canal izquierdo |
| 0x0C | RIGHTDATA | R/W | dato del canal derecho |
### Desglose del registro FIFOSPACE (32 bits)
| Bits | Campo | Significado |
|------|-------|-------------|
| [7:0] | RARC | Read Available Right Channel |
| [15:8] | RALC | Read Available Left Channel |
| [23:16] | WSRC | Write Space Right Channel |
| [31:24] | WSLC | Write Space Left Channel |

Antes de reproducir, verificar que WSLC/WSRC > 0 antes de escribir samples.

### Ejemplo de uso (NIOS II Bare Metal)
```c
#include "memory_map.h"
/* Escribir un sample estéreo si hay espacio en el FIFO */
uint32_t space = REG_READ(AUDIO_BASE, AUDIO_FIFOSPACE_OFFSET);
if (AUDIO_FIFO_WSLC(space) > 0) {
    REG_WRITE(AUDIO_BASE, AUDIO_LEFTDATA_OFFSET,  sample_left);
    REG_WRITE(AUDIO_BASE, AUDIO_RIGHTDATA_OFFSET, sample_right);
}
```
## Audio Config
Audio and Video Config (altera_up_avalon_audio_and_video_config).
Configura el codec WM8731 por I2C automáticamente al arrancar
(Auto Initialize habilitado). Normalmente no requiere acceso manual.
## Registros de los PIOs

Cada PIO tiene 4 registros consecutivos de 32 bits:

| Offset | Nombre | R/W | Función |
|--------|--------|-----|---------|
| 0x00 | DATA | R/W | Valor actual de los pines |
| 0x04 | DIRECTION | R/W | Dirección (no usado en input-only / output-only) |
| 0x08 | INTERRUPTMASK | R/W | Máscara de interrupciones (solo buttons) |
| 0x0C | EDGECAPTURE | R/W | Captura de flancos (solo buttons) |

## Registros del Interval Timer

| Offset | Nombre | R/W | Función |
|--------|--------|-----|---------|
| 0x00 | STATUS | R/W | bit 0: TO (timeout), bit 1: RUN |
| 0x04 | CONTROL | R/W | bit 0: ITO (IRQ enable), bit 1: CONT, bit 2: START, bit 3: STOP |
| 0x08 | PERIODL | R/W | Período (16 bits bajos) |
| 0x0C | PERIODH | R/W | Período (16 bits altos) |
| 0x10 | SNAPL | R/W | Snapshot (16 bits bajos) |
| 0x14 | SNAPH | R/W | Snapshot (16 bits altos) |

## Vectors del NIOS
- **Reset vector**: `0x0000_0000` (en RAM)
- **Exception vector**: `0x0000_0020` (en RAM)

## Espacio de direcciones reservado para expansión

| Rango | Uso futuro propuesto |
|-------|----------------------|
| `0x0007_1000` | Libre (entre Audio IP y Audio Config) |
| `0x0008_0000+` | IPC con HPS (REQ-17) |
| `0x0009_0000+` | Controlador VGA/LCD (REQ-09) |
---
