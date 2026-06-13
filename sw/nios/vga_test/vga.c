/*
 * vga.c
 *
 * Implementacion del driver VGA (Character Buffer for VGA Display).
 * Ver vga.h para la descripcion de la interfaz.
 *
 * NOTA DE DIRECCIONAMIENTO:
 *   El Character Buffer expone una posicion por CADA BYTE:
 *     direccion = CHAR_BUFFER_BASE + (y << 7) + x
 *   Cada caracter es 1 byte (codigo ASCII). Por eso el puntero de
 *   acceso es (volatile char *), NO (volatile uint32_t *). Usar un
 *   puntero de 32 bits hacia que cada caracter cayera cada 4 columnas
 *   (texto espaciado/entremezclado en pantalla).
 *
 *   El rango del slave (0x9_4000..0x9_5fff = 8 KB) confirma acceso por
 *   byte: 60 filas * 128 (stride 1<<7) = 7680 bytes, que caben en 8 KB.
 *   Si fuera acceso por word de 32 bits requeriria 4x ese espacio.
 *
 * R_SoC - REQ-09
 */

#include "vga.h"

/* Calcula la direccion de memoria del caracter en (x, y).
 *   X ocupa los bits [6:0], Y los bits [12:7].
 *   Por eso el offset de fila es (y << 7).
 *   Acceso por BYTE: puntero a char (1 byte por caracter). */
static inline volatile char *vga_addr(int x, int y)
{
    return (volatile char *)(CHAR_BUFFER_BASE + ((uint32_t)y << 7) + (uint32_t)x);
}

void vga_putchar(int x, int y, char c)
{
    if (x < 0 || x >= VGA_COLS || y < 0 || y >= VGA_ROWS) {
        return;  /* fuera de la grilla, ignorar */
    }
    *vga_addr(x, y) = c;
}

void vga_print(int x, int y, const char *str)
{
    if (y < 0 || y >= VGA_ROWS) {
        return;
    }
    while (*str != '\0' && x < VGA_COLS) {
        if (x >= 0) {
            *vga_addr(x, y) = *str;
        }
        x++;
        str++;
    }
}

void vga_print_centered(int y, const char *str)
{
    /* medir la longitud de la cadena */
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    int x = (VGA_COLS - len) / 2;
    if (x < 0) {
        x = 0;
    }
    vga_print(x, y, str);
}

void vga_clear(void)
{
    int x, y;
    for (y = 0; y < VGA_ROWS; y++) {
        for (x = 0; x < VGA_COLS; x++) {
            *vga_addr(x, y) = ' ';
        }
    }
}

void vga_clear_row(int y)
{
    int x;
    if (y < 0 || y >= VGA_ROWS) {
        return;
    }
    for (x = 0; x < VGA_COLS; x++) {
        *vga_addr(x, y) = ' ';
    }
}

int vga_print_uint(int x, int y, uint32_t value)
{
    char buf[11];   /* uint32 maximo: 4294967295 = 10 digitos + '\0' */
    int i = 0;
    int j;

    /* caso especial: cero */
    if (value == 0) {
        vga_putchar(x, y, '0');
        return 1;
    }

    /* extraer digitos en orden inverso */
    while (value > 0 && i < 10) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    /* escribir en orden correcto */
    for (j = 0; j < i; j++) {
        vga_putchar(x + j, y, buf[i - 1 - j]);
    }
    return i;
}

void vga_print_time(int x, int y, uint32_t total_seconds)
{
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;

    /* limitar minutos a 2 digitos visibles (99 max) */
    if (minutes > 99) {
        minutes = 99;
    }

    vga_putchar(x + 0, y, (char)('0' + (minutes / 10)));
    vga_putchar(x + 1, y, (char)('0' + (minutes % 10)));
    vga_putchar(x + 2, y, ':');
    vga_putchar(x + 3, y, (char)('0' + (seconds / 10)));
    vga_putchar(x + 4, y, (char)('0' + (seconds % 10)));
}
