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
#define AUDIO_BASE            0x00070000U   /* Audio IP (reproducción) */
#define AUDIO_CONFIG_BASE     0x00072000U   /* Config I2C del codec WM8731 */
#define AUDIO_FILTER_CONTROL_BASE  0x00080000U  /* Selección de filtro (R_DSP) */
#define AUDIO_SAMPLE_INPUT_BASE    0x00081000U  /* FIFO de muestras (R_DSP) */
/* ==========================================================
 * IRQs del NIOS II
 * ========================================================== */
#define IRQ_JTAG_UART         1
#define IRQ_TIMER             2
#define IRQ_BUTTONS           3
#define IRQ_AUDIO             4
/* ==========================================================
 * HEX Display Controller (REQ-04, REQ-10)
 *
 * Módulo personalizado que controla los 6 displays de 7-seg.
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
 * Audio IP (altera_up_avalon_audio)
 *
 * Audio IP de Intel University Program.
 * Solo reproducción (Audio Out), 16-bit, 48 kHz.
 * ========================================================== */
/* Offsets de registros */
#define AUDIO_CONTROL_OFFSET   0x00   /* enable IRQ, clear FIFOs */
#define AUDIO_FIFOSPACE_OFFSET 0x04   /* espacio disponible en FIFOs */
#define AUDIO_LEFTDATA_OFFSET  0x08   /* dato canal izquierdo */
#define AUDIO_RIGHTDATA_OFFSET 0x0C   /* dato canal derecho */
/* Bits del registro CONTROL */
#define AUDIO_CTRL_RE          (1 << 0)   /* Read interrupt enable */
#define AUDIO_CTRL_WE          (1 << 1)   /* Write interrupt enable */
#define AUDIO_CTRL_CR          (1 << 2)   /* Clear read FIFO */
#define AUDIO_CTRL_CW          (1 << 3)   /* Clear write FIFO */
/* FIFOSPACE: 4 bytes empaquetados en un registro de 32 bits
 *   [7:0]   RARC - Read Available Right Channel
 *   [15:8]  RALC - Read Available Left Channel
 *   [23:16] WSRC - Write Space Right Channel
 *   [31:24] WSLC - Write Space Left Channel
 * Antes de reproducir, verificar que WSLC/WSRC > 0. */
#define AUDIO_FIFO_WSLC(x)     (((x) >> 24) & 0xFF)
#define AUDIO_FIFO_WSRC(x)     (((x) >> 16) & 0xFF)
#define AUDIO_FIFO_RALC(x)     (((x) >> 8)  & 0xFF)
#define AUDIO_FIFO_RARC(x)     (((x) >> 0)  & 0xFF)
/* ==========================================================
 * Audio Config (altera_up_avalon_audio_and_video_config)
 *
 * Configura el codec WM8731 por I2C al arrancar (Auto Initialize).
 * Normalmente no se requiere acceso manual a sus registros.
 * ========================================================== */
/* ==========================================================
 * Audio Filter Control (R_DSP - Noemi)
 *
 * Registro de selección de filtro. El NIOS lee los switches,
 * decide el filtro y escribe filter_sel aquí. El conduit
 * filter_sel (2 bits) va al módulo AudioFilter en el top-level.
 *
 * filter_sel:
 *   0 = bypass, 1 = lowpass, 2 = highpass, 3 = bass boost
 *
 * NOTA: addressUnits = WORDS. address N -> offset byte N*4.
 * ========================================================== */
/* Offsets de registros */
#define FILTER_CONTROL_OFFSET  0x00   /* R/W: bits[1:0] = filter_sel */
#define FILTER_STATUS_OFFSET   0x04   /* R:   bits[1:0] = filter_sel actual */
#define FILTER_ID_OFFSET       0x08   /* R:   0xA0F10001 (ID/debug) */
/* Valores de selección de filtro */
#define FILTER_SEL_BYPASS      0x0
#define FILTER_SEL_LOWPASS     0x1
#define FILTER_SEL_HIGHPASS    0x2
#define FILTER_SEL_BASSBOOST   0x3
/* ==========================================================
 * Audio Sample Input (R_DSP - Noemi)
 *
 * FIFO de doble reloj. El NIOS escribe muestras PCM 16-bit que
 * salen por el conduit hacia AudioFilter -> serializador -> codec.
 *
 * NOTA: addressUnits = WORDS. address N -> offset byte N*4.
 * ========================================================== */
/* Offsets de registros */
#define SAMPLE_WRITE_OFFSET    0x00   /* W: bits[15:0] = muestra PCM signed 16-bit */
#define SAMPLE_STATUS_OFFSET   0x04   /* R: flags de estado del FIFO (ver bits) */
#define SAMPLE_CONTROL_OFFSET  0x08   /* R/W: bit 0 = enable */
#define SAMPLE_ID_OFFSET       0x0C   /* R: 0xA5A10001 (ID/debug) */
/* Bits del registro STATUS */
#define SAMPLE_STATUS_FIFO_FULL    (1 << 0)
#define SAMPLE_STATUS_FIFO_EMPTY   (1 << 1)
#define SAMPLE_STATUS_READY_WR     (1 << 2)   /* listo para escribir */
#define SAMPLE_STATUS_OVERFLOW     (1 << 3)
#define SAMPLE_STATUS_UNDERFLOW    (1 << 4)
/* bits [25:16] del STATUS = wrusedw (nivel de llenado aproximado) */
#define SAMPLE_STATUS_WRUSEDW(x)   (((x) >> 16) & 0x3FF)
/* Bits del registro CONTROL */
#define SAMPLE_CTRL_ENABLE     (1 << 0)
#define SAMPLE_CTRL_CLEAR      (1 << 1)   /* writedata[1]=1 limpia flags */
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
 *
 *   // Seleccionar filtro según switches:
 *   uint32_t sw = REG_READ(SWITCHES_PIO_BASE, PIO_DATA_OFFSET) & 0x3;
 *   REG_WRITE(AUDIO_FILTER_CONTROL_BASE, FILTER_CONTROL_OFFSET, sw);
 *
 *   // Escribir una muestra al FIFO si hay espacio:
 *   uint32_t st = REG_READ(AUDIO_SAMPLE_INPUT_BASE, SAMPLE_STATUS_OFFSET);
 *   if (!(st & SAMPLE_STATUS_FIFO_FULL)) {
 *       REG_WRITE(AUDIO_SAMPLE_INPUT_BASE, SAMPLE_WRITE_OFFSET, muestra & 0xFFFF);
 *   }
 * ========================================================== */
#define REG_WRITE(base, offset, value) \
    (*((volatile uint32_t *)((base) + (offset))) = (value))
#define REG_READ(base, offset) \
    (*((volatile uint32_t *)((base) + (offset))))
#endif /* MEMORY_MAP_H */