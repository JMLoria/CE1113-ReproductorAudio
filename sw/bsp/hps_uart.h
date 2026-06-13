#ifndef HPS_UART_H
#define HPS_UART_H

/* Salida serie por la UART0 del HPS (DE1-SoC: conectada al USB-UART).
 * U-Boot ya la dejó configurada a 115200 8N1, así que solo transmitimos. */

void uart_putc(char c);
void uart_puts(const char* s);

#endif /* HPS_UART_H */
