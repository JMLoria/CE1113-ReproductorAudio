/*
 * memory_map.h
 *
 * Mapa de memoria del sistema NIOS II para el reproductor de audio.
 *
 * REQ-16: acceso por punteros directos, sin uso de HAL.
 *
 * Última actualización: 2026-05-30
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

/* ==========================================================
 * IRQs del NIOS II
 * ========================================================== */

#define IRQ_JTAG_UART         1
#define IRQ_TIMER             2
#define IRQ_BUTTONS           3

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
 *   REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, 0xFF);
 *   uint32_t v = REG_READ(SWITCHES_PIO_BASE, PIO_DATA_OFFSET);
 * ========================================================== */

#define REG_WRITE(base, offset, value) \
    (*((volatile uint32_t *)((base) + (offset))) = (value))

#define REG_READ(base, offset) \
    (*((volatile uint32_t *)((base) + (offset))))

#endif /* MEMORY_MAP_H */