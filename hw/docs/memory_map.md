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

## IRQs del NIOS II

| IRQ | Componente |
|-----|------------|
| 1 | JTAG UART |
| 2 | Interval Timer |
| 3 | Buttons PIO |

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
| `0x0007_0000+` | Filtros digitales (REQ-05, R_DSP) |
| `0x0008_0000+` | Audio IP / IPC con HPS (REQ-17) |
| `0x0009_0000+` | Controlador VGA/LCD (REQ-09) |

---
