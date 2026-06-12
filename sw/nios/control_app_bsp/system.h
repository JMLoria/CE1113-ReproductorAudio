/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'nios2' in SOPC Builder design 'soc_system'
 * SOPC Builder design path: ../../../hw/soc_system.sopcinfo
 *
 * Generated: Thu Jun 11 22:00:43 CST 2026
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x00010020
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "tiny"
#define ALT_CPU_DATA_ADDR_WIDTH 0x14
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_EXCEPTION_ADDR 0x00000020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 50000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 0
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 0
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_ICACHE_SIZE 0
#define ALT_CPU_INST_ADDR_WIDTH 0x11
#define ALT_CPU_NAME "nios2"
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x00000000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x00010020
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "tiny"
#define NIOS2_DATA_ADDR_WIDTH 0x14
#define NIOS2_DCACHE_LINE_SIZE 0
#define NIOS2_DCACHE_LINE_SIZE_LOG2 0
#define NIOS2_DCACHE_SIZE 0
#define NIOS2_EXCEPTION_ADDR 0x00000020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 0
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 0
#define NIOS2_ICACHE_LINE_SIZE_LOG2 0
#define NIOS2_ICACHE_SIZE 0
#define NIOS2_INST_ADDR_WIDTH 0x11
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x00000000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_FIFO
#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SYSID_QSYS
#define __ALTERA_AVALON_TIMER
#define __ALTERA_NIOS2_GEN2
#define __ALTERA_UP_AVALON_AUDIO
#define __ALTERA_UP_AVALON_AUDIO_AND_VIDEO_CONFIG
#define __ALTERA_UP_AVALON_VIDEO_CHARACTER_BUFFER_WITH_DMA
#define __AUDIO_FILTER_CONTROL
#define __AUDIO_SAMPLE_INPUT
#define __HEX_DISPLAY_CONTROLLER


/*
 * RAM configuration
 *
 */

#define ALT_MODULE_CLASS_RAM altera_avalon_onchip_memory2
#define RAM_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define RAM_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define RAM_BASE 0x0
#define RAM_CONTENTS_INFO ""
#define RAM_DUAL_PORT 0
#define RAM_GUI_RAM_BLOCK_TYPE "AUTO"
#define RAM_INIT_CONTENTS_FILE "soc_system_RAM"
#define RAM_INIT_MEM_CONTENT 1
#define RAM_INSTANCE_ID "NONE"
#define RAM_IRQ -1
#define RAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define RAM_NAME "/dev/RAM"
#define RAM_NON_DEFAULT_INIT_FILE_ENABLED 0
#define RAM_RAM_BLOCK_TYPE "AUTO"
#define RAM_READ_DURING_WRITE_MODE "DONT_CARE"
#define RAM_SINGLE_CLOCK_OP 0
#define RAM_SIZE_MULTIPLE 1
#define RAM_SIZE_VALUE 65536
#define RAM_SPAN 65536
#define RAM_TYPE "altera_avalon_onchip_memory2"
#define RAM_WRITABLE 1


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "Cyclone V"
#define ALT_IRQ_BASE NULL
#define ALT_LEGACY_INTERRUPT_API_PRESENT
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/jtag_uart"
#define ALT_STDERR_BASE 0x30000
#define ALT_STDERR_DEV jtag_uart
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/jtag_uart"
#define ALT_STDIN_BASE 0x30000
#define ALT_STDIN_DEV jtag_uart
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/jtag_uart"
#define ALT_STDOUT_BASE 0x30000
#define ALT_STDOUT_DEV jtag_uart
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "soc_system"


/*
 * audio configuration
 *
 */

#define ALT_MODULE_CLASS_audio altera_up_avalon_audio
#define AUDIO_BASE 0x70000
#define AUDIO_IRQ 4
#define AUDIO_IRQ_INTERRUPT_CONTROLLER_ID 0
#define AUDIO_NAME "/dev/audio"
#define AUDIO_SPAN 16
#define AUDIO_TYPE "altera_up_avalon_audio"


/*
 * audio_config configuration
 *
 */

#define ALT_MODULE_CLASS_audio_config altera_up_avalon_audio_and_video_config
#define AUDIO_CONFIG_BASE 0x72000
#define AUDIO_CONFIG_IRQ -1
#define AUDIO_CONFIG_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AUDIO_CONFIG_NAME "/dev/audio_config"
#define AUDIO_CONFIG_SPAN 16
#define AUDIO_CONFIG_TYPE "altera_up_avalon_audio_and_video_config"


/*
 * audio_filter_control configuration
 *
 */

#define ALT_MODULE_CLASS_audio_filter_control audio_filter_control
#define AUDIO_FILTER_CONTROL_BASE 0x80000
#define AUDIO_FILTER_CONTROL_IRQ -1
#define AUDIO_FILTER_CONTROL_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AUDIO_FILTER_CONTROL_NAME "/dev/audio_filter_control"
#define AUDIO_FILTER_CONTROL_SPAN 16
#define AUDIO_FILTER_CONTROL_TYPE "audio_filter_control"


/*
 * audio_sample_input configuration
 *
 */

#define ALT_MODULE_CLASS_audio_sample_input audio_sample_input
#define AUDIO_SAMPLE_INPUT_BASE 0x81000
#define AUDIO_SAMPLE_INPUT_IRQ -1
#define AUDIO_SAMPLE_INPUT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AUDIO_SAMPLE_INPUT_NAME "/dev/audio_sample_input"
#define AUDIO_SAMPLE_INPUT_SPAN 16
#define AUDIO_SAMPLE_INPUT_TYPE "audio_sample_input"


/*
 * buttons_pio configuration
 *
 */

#define ALT_MODULE_CLASS_buttons_pio altera_avalon_pio
#define BUTTONS_PIO_BASE 0x50000
#define BUTTONS_PIO_BIT_CLEARING_EDGE_REGISTER 1
#define BUTTONS_PIO_BIT_MODIFYING_OUTPUT_REGISTER 0
#define BUTTONS_PIO_CAPTURE 1
#define BUTTONS_PIO_DATA_WIDTH 4
#define BUTTONS_PIO_DO_TEST_BENCH_WIRING 0
#define BUTTONS_PIO_DRIVEN_SIM_VALUE 0
#define BUTTONS_PIO_EDGE_TYPE "FALLING"
#define BUTTONS_PIO_FREQ 50000000
#define BUTTONS_PIO_HAS_IN 1
#define BUTTONS_PIO_HAS_OUT 0
#define BUTTONS_PIO_HAS_TRI 0
#define BUTTONS_PIO_IRQ 3
#define BUTTONS_PIO_IRQ_INTERRUPT_CONTROLLER_ID 0
#define BUTTONS_PIO_IRQ_TYPE "EDGE"
#define BUTTONS_PIO_NAME "/dev/buttons_pio"
#define BUTTONS_PIO_RESET_VALUE 0
#define BUTTONS_PIO_SPAN 16
#define BUTTONS_PIO_TYPE "altera_avalon_pio"


/*
 * char_buffer_avalon_char_buffer_slave configuration
 *
 */

#define ALT_MODULE_CLASS_char_buffer_avalon_char_buffer_slave altera_up_avalon_video_character_buffer_with_dma
#define CHAR_BUFFER_AVALON_CHAR_BUFFER_SLAVE_BASE 0x94000
#define CHAR_BUFFER_AVALON_CHAR_BUFFER_SLAVE_IRQ -1
#define CHAR_BUFFER_AVALON_CHAR_BUFFER_SLAVE_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CHAR_BUFFER_AVALON_CHAR_BUFFER_SLAVE_NAME "/dev/char_buffer_avalon_char_buffer_slave"
#define CHAR_BUFFER_AVALON_CHAR_BUFFER_SLAVE_SPAN 8192
#define CHAR_BUFFER_AVALON_CHAR_BUFFER_SLAVE_TYPE "altera_up_avalon_video_character_buffer_with_dma"


/*
 * char_buffer_avalon_char_control_slave configuration
 *
 */

#define ALT_MODULE_CLASS_char_buffer_avalon_char_control_slave altera_up_avalon_video_character_buffer_with_dma
#define CHAR_BUFFER_AVALON_CHAR_CONTROL_SLAVE_BASE 0x90000
#define CHAR_BUFFER_AVALON_CHAR_CONTROL_SLAVE_IRQ -1
#define CHAR_BUFFER_AVALON_CHAR_CONTROL_SLAVE_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CHAR_BUFFER_AVALON_CHAR_CONTROL_SLAVE_NAME "/dev/char_buffer_avalon_char_control_slave"
#define CHAR_BUFFER_AVALON_CHAR_CONTROL_SLAVE_SPAN 8
#define CHAR_BUFFER_AVALON_CHAR_CONTROL_SLAVE_TYPE "altera_up_avalon_video_character_buffer_with_dma"


/*
 * fifo_hps_nios_in_csr configuration
 *
 */

#define ALT_MODULE_CLASS_fifo_hps_nios_in_csr altera_avalon_fifo
#define FIFO_HPS_NIOS_IN_CSR_AVALONMM_AVALONMM_DATA_WIDTH 32
#define FIFO_HPS_NIOS_IN_CSR_AVALONMM_AVALONST_DATA_WIDTH 32
#define FIFO_HPS_NIOS_IN_CSR_BASE 0xa0020
#define FIFO_HPS_NIOS_IN_CSR_BITS_PER_SYMBOL 16
#define FIFO_HPS_NIOS_IN_CSR_CHANNEL_WIDTH 8
#define FIFO_HPS_NIOS_IN_CSR_ERROR_WIDTH 8
#define FIFO_HPS_NIOS_IN_CSR_FIFO_DEPTH 1024
#define FIFO_HPS_NIOS_IN_CSR_IRQ -1
#define FIFO_HPS_NIOS_IN_CSR_IRQ_INTERRUPT_CONTROLLER_ID -1
#define FIFO_HPS_NIOS_IN_CSR_NAME "/dev/fifo_hps_nios_in_csr"
#define FIFO_HPS_NIOS_IN_CSR_SINGLE_CLOCK_MODE 1
#define FIFO_HPS_NIOS_IN_CSR_SPAN 32
#define FIFO_HPS_NIOS_IN_CSR_SYMBOLS_PER_BEAT 2
#define FIFO_HPS_NIOS_IN_CSR_TYPE "altera_avalon_fifo"
#define FIFO_HPS_NIOS_IN_CSR_USE_AVALONMM_READ_SLAVE 1
#define FIFO_HPS_NIOS_IN_CSR_USE_AVALONMM_WRITE_SLAVE 1
#define FIFO_HPS_NIOS_IN_CSR_USE_AVALONST_SINK 0
#define FIFO_HPS_NIOS_IN_CSR_USE_AVALONST_SOURCE 0
#define FIFO_HPS_NIOS_IN_CSR_USE_BACKPRESSURE 1
#define FIFO_HPS_NIOS_IN_CSR_USE_IRQ 1
#define FIFO_HPS_NIOS_IN_CSR_USE_PACKET 1
#define FIFO_HPS_NIOS_IN_CSR_USE_READ_CONTROL 0
#define FIFO_HPS_NIOS_IN_CSR_USE_REGISTER 0
#define FIFO_HPS_NIOS_IN_CSR_USE_WRITE_CONTROL 1


/*
 * fifo_hps_nios_out configuration
 *
 */

#define ALT_MODULE_CLASS_fifo_hps_nios_out altera_avalon_fifo
#define FIFO_HPS_NIOS_OUT_AVALONMM_AVALONMM_DATA_WIDTH 32
#define FIFO_HPS_NIOS_OUT_AVALONMM_AVALONST_DATA_WIDTH 32
#define FIFO_HPS_NIOS_OUT_BASE 0xa0000
#define FIFO_HPS_NIOS_OUT_BITS_PER_SYMBOL 16
#define FIFO_HPS_NIOS_OUT_CHANNEL_WIDTH 8
#define FIFO_HPS_NIOS_OUT_ERROR_WIDTH 8
#define FIFO_HPS_NIOS_OUT_FIFO_DEPTH 1024
#define FIFO_HPS_NIOS_OUT_IRQ -1
#define FIFO_HPS_NIOS_OUT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define FIFO_HPS_NIOS_OUT_NAME "/dev/fifo_hps_nios_out"
#define FIFO_HPS_NIOS_OUT_SINGLE_CLOCK_MODE 1
#define FIFO_HPS_NIOS_OUT_SPAN 4
#define FIFO_HPS_NIOS_OUT_SYMBOLS_PER_BEAT 2
#define FIFO_HPS_NIOS_OUT_TYPE "altera_avalon_fifo"
#define FIFO_HPS_NIOS_OUT_USE_AVALONMM_READ_SLAVE 1
#define FIFO_HPS_NIOS_OUT_USE_AVALONMM_WRITE_SLAVE 1
#define FIFO_HPS_NIOS_OUT_USE_AVALONST_SINK 0
#define FIFO_HPS_NIOS_OUT_USE_AVALONST_SOURCE 0
#define FIFO_HPS_NIOS_OUT_USE_BACKPRESSURE 1
#define FIFO_HPS_NIOS_OUT_USE_IRQ 1
#define FIFO_HPS_NIOS_OUT_USE_PACKET 1
#define FIFO_HPS_NIOS_OUT_USE_READ_CONTROL 0
#define FIFO_HPS_NIOS_OUT_USE_REGISTER 0
#define FIFO_HPS_NIOS_OUT_USE_WRITE_CONTROL 1


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 32
#define ALT_SYS_CLK TIMER
#define ALT_TIMESTAMP_CLK none


/*
 * hex_display configuration
 *
 */

#define ALT_MODULE_CLASS_hex_display hex_display_controller
#define HEX_DISPLAY_BASE 0x60000
#define HEX_DISPLAY_IRQ -1
#define HEX_DISPLAY_IRQ_INTERRUPT_CONTROLLER_ID -1
#define HEX_DISPLAY_NAME "/dev/hex_display"
#define HEX_DISPLAY_SPAN 16
#define HEX_DISPLAY_TYPE "hex_display_controller"


/*
 * jtag_uart configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart altera_avalon_jtag_uart
#define JTAG_UART_BASE 0x30000
#define JTAG_UART_IRQ 1
#define JTAG_UART_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_NAME "/dev/jtag_uart"
#define JTAG_UART_READ_DEPTH 64
#define JTAG_UART_READ_THRESHOLD 8
#define JTAG_UART_SPAN 8
#define JTAG_UART_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_WRITE_DEPTH 64
#define JTAG_UART_WRITE_THRESHOLD 8


/*
 * leds_pio configuration
 *
 */

#define ALT_MODULE_CLASS_leds_pio altera_avalon_pio
#define LEDS_PIO_BASE 0x52000
#define LEDS_PIO_BIT_CLEARING_EDGE_REGISTER 0
#define LEDS_PIO_BIT_MODIFYING_OUTPUT_REGISTER 0
#define LEDS_PIO_CAPTURE 0
#define LEDS_PIO_DATA_WIDTH 10
#define LEDS_PIO_DO_TEST_BENCH_WIRING 0
#define LEDS_PIO_DRIVEN_SIM_VALUE 0
#define LEDS_PIO_EDGE_TYPE "NONE"
#define LEDS_PIO_FREQ 50000000
#define LEDS_PIO_HAS_IN 0
#define LEDS_PIO_HAS_OUT 1
#define LEDS_PIO_HAS_TRI 0
#define LEDS_PIO_IRQ -1
#define LEDS_PIO_IRQ_INTERRUPT_CONTROLLER_ID -1
#define LEDS_PIO_IRQ_TYPE "NONE"
#define LEDS_PIO_NAME "/dev/leds_pio"
#define LEDS_PIO_RESET_VALUE 0
#define LEDS_PIO_SPAN 16
#define LEDS_PIO_TYPE "altera_avalon_pio"


/*
 * switches_pio configuration
 *
 */

#define ALT_MODULE_CLASS_switches_pio altera_avalon_pio
#define SWITCHES_PIO_BASE 0x51000
#define SWITCHES_PIO_BIT_CLEARING_EDGE_REGISTER 0
#define SWITCHES_PIO_BIT_MODIFYING_OUTPUT_REGISTER 0
#define SWITCHES_PIO_CAPTURE 0
#define SWITCHES_PIO_DATA_WIDTH 10
#define SWITCHES_PIO_DO_TEST_BENCH_WIRING 0
#define SWITCHES_PIO_DRIVEN_SIM_VALUE 0
#define SWITCHES_PIO_EDGE_TYPE "NONE"
#define SWITCHES_PIO_FREQ 50000000
#define SWITCHES_PIO_HAS_IN 1
#define SWITCHES_PIO_HAS_OUT 0
#define SWITCHES_PIO_HAS_TRI 0
#define SWITCHES_PIO_IRQ -1
#define SWITCHES_PIO_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SWITCHES_PIO_IRQ_TYPE "NONE"
#define SWITCHES_PIO_NAME "/dev/switches_pio"
#define SWITCHES_PIO_RESET_VALUE 0
#define SWITCHES_PIO_SPAN 16
#define SWITCHES_PIO_TYPE "altera_avalon_pio"


/*
 * sysid configuration
 *
 */

#define ALT_MODULE_CLASS_sysid altera_avalon_sysid_qsys
#define SYSID_BASE 0x20000
#define SYSID_ID -2147374262
#define SYSID_IRQ -1
#define SYSID_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SYSID_NAME "/dev/sysid"
#define SYSID_SPAN 8
#define SYSID_TIMESTAMP 1781235147
#define SYSID_TYPE "altera_avalon_sysid_qsys"


/*
 * timer configuration
 *
 */

#define ALT_MODULE_CLASS_timer altera_avalon_timer
#define TIMER_ALWAYS_RUN 0
#define TIMER_BASE 0x40000
#define TIMER_COUNTER_SIZE 32
#define TIMER_FIXED_PERIOD 0
#define TIMER_FREQ 50000000
#define TIMER_IRQ 2
#define TIMER_IRQ_INTERRUPT_CONTROLLER_ID 0
#define TIMER_LOAD_VALUE 49999
#define TIMER_MULT 0.001
#define TIMER_NAME "/dev/timer"
#define TIMER_PERIOD 1
#define TIMER_PERIOD_UNITS "ms"
#define TIMER_RESET_OUTPUT 0
#define TIMER_SNAPSHOT 1
#define TIMER_SPAN 32
#define TIMER_TICKS_PER_SEC 1000
#define TIMER_TIMEOUT_PULSE_OUTPUT 0
#define TIMER_TYPE "altera_avalon_timer"

#endif /* __SYSTEM_H_ */
