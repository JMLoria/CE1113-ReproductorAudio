/*
 * memory_map.h
 *
 * Mapa de memoria del sistema NIOS II para el reproductor de audio.
 *
 * Acceso por punteros directos, sin uso de HAL.
 *
 */

#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <stdint.h>

/* ==========================================================
 * Direcciones base de periféricos (vista desde NIOS II)
 * ========================================================== */

#define RAM_BASE              0x00000000U   /* 64 KB on-chip RAM */
#define NIOS_DEBUG_BASE       0x00010000U
#define SYSID_BASE            0x00020000U
#define JTAG_UART_BASE        0x00030000U
#define TIMER_BASE            0x00040000U
#define BUTTONS_PIO_BASE      0x00050000U
#define SWITCHES_PIO_BASE     0x00051000U
#define LEDS_PIO_BASE         0x00052000U
#define HEX_DISPLAY_BASE      0x00060000U   /* módulo personalizado */

/* ==========================================================
 * IRQs del NIOS II
 * ========================================================== */

#define IRQ_JTAG_UART         1
#define IRQ_TIMER             2
#define IRQ_BUTTONS           3

/* ==========================================================
 * HEX Display Controller (REQ-04, REQ-10)
 *
 * Módulo personalizado que controla los 6 displays de 7-seg.
 * Diseñado por R_SoC.
 * ========================================================== */

/* Offsets de registros */
#define HEX_CONTROL_OFFSET    0x00
#define HEX_STATUS_OFFSET     0x04
#define HEX_DATA_OFFSET       0x08

/* Bits del registro CONTROL */
#define HEX_CTRL_ENABLE       (1 << 0)
#define HEX_CTRL_MODE_TIME    (0 << 1)   /* mostrar MM:SS */
#define HEX_CTRL_MODE_HEX     (1 << 1)   /* mostrar número hex */

/* Bits del registro STATUS */
#define HEX_STATUS_READY      (1 << 0)

/* ==========================================================
 * Offsets de registros - PIOs (Parallel I/O)
 * ========================================================== */

#define PIO_DATA_OFFSET       0x00
#define PIO_DIRECTION_OFFSET  0x04
#define PIO_IRQMASK_OFFSET    0x08
#define PIO_EDGECAP_OFFSET    0x0C

/* ==========================================================
 * Offsets de registros - Interval Timer
 * ========================================================== */

#define TIMER_STATUS_OFFSET   0x00
#define TIMER_CONTROL_OFFSET  0x04
#define TIMER_PERIODL_OFFSET  0x08
#define TIMER_PERIODH_OFFSET  0x0C
#define TIMER_SNAPL_OFFSET    0x10
#define TIMER_SNAPH_OFFSET    0x14

/* Bits del Timer Control */
#define TIMER_CTRL_ITO        (1 << 0)   /* IRQ enable */
#define TIMER_CTRL_CONT       (1 << 1)   /* Continuous */
#define TIMER_CTRL_START      (1 << 2)
#define TIMER_CTRL_STOP       (1 << 3)

/* Bits del Timer Status */
#define TIMER_STATUS_TO       (1 << 0)   /* Timeout flag */
#define TIMER_STATUS_RUN      (1 << 1)

/* ==========================================================
 * Macros para acceso a registros (REG_READ / REG_WRITE)
 *
 * Uso:
 *   REG_WRITE(HEX_DISPLAY_BASE, HEX_CONTROL_OFFSET, HEX_CTRL_ENABLE);
 *   REG_WRITE(HEX_DISPLAY_BASE, HEX_DATA_OFFSET, 125);  // mostrar 02:05
 *
 *   REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, 0xFF);
 *   uint32_t sw = REG_READ(SWITCHES_PIO_BASE, PIO_DATA_OFFSET);
 * ========================================================== */

#define REG_WRITE(base, offset, value) \
    (*((volatile uint32_t *)((base) + (offset))) = (value))

#define REG_READ(base, offset) \
    (*((volatile uint32_t *)((base) + (offset))))

#endif /* MEMORY_MAP_H */