#include "hps_uart.h"
#include <stdint.h>

/* =============================================================================
 * UART0 del HPS Cyclone V (Synopsys DW APB UART, registros de 32 bits).
 * Base 0xFFC02000 en la DE1-SoC, ruteada al puente USB-UART de la placa.
 * No la reinicializamos: U-Boot ya la dejó a 115200 8N1; solo transmitimos
 * usando el bit THRE (Transmit Holding Register Empty) del LSR.
 * ========================================================================== */
#define UART0_BASE   0xFFC02000u
#define UART_THR     (*(volatile uint32_t*)(UART0_BASE + 0x00))  /* TX holding */
#define UART_LSR     (*(volatile uint32_t*)(UART0_BASE + 0x14))  /* Line Status */
#define LSR_THRE     (1u << 5)                                   /* THR vacío   */

void uart_putc(char c) {
    while (!(UART_LSR & LSR_THRE)) { /* esperar a que el THR se vacíe */ }
    UART_THR = (uint32_t)(unsigned char)c;
}

void uart_puts(const char* s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r');   /* CRLF para terminales serie */
        uart_putc(*s++);
    }
}
