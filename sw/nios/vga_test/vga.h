/*
 * vga.h
 *
 * Driver para el Character Buffer for VGA Display (Intel UP).
 * Modo texto, grilla 80x60 caracteres, resolucion 640x480.
 *
 * El NIOS escribe codigos ASCII y el hardware renderiza la fuente.
 * Texto en blanco sobre fondo transparente. El hardware limpia la
 * pantalla automaticamente al reset.
 *
 * Direccionamiento de un caracter en (x, y):
 *   direccion = CHAR_BUFFER_BASE + (y << 7) + x
 *   X = bits [6:0] (0..79), Y = bits [12:7] (0..59)
 *   Acceso por palabra de 32 bits (el ASCII va en los bits bajos).
 *
 * R_SoC - REQ-09
 */
#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include "memory_map.h"

/* Dimensiones de la grilla de caracteres */
#define VGA_COLS 80
#define VGA_ROWS 60

/* Escribe un caracter 'c' en la posicion (x, y).
 * Si (x, y) esta fuera de la grilla, no hace nada. */
void vga_putchar(int x, int y, char c);

/* Escribe la cadena 'str' empezando en (x, y), avanzando a la
 * derecha. No hace wrap de linea: corta al llegar al borde. */
void vga_print(int x, int y, const char *str);

/* Escribe 'str' centrada horizontalmente en la fila 'y'. */
void vga_print_centered(int y, const char *str);

/* Limpia toda la pantalla (escribe espacios en toda la grilla).
 * Nota: el hardware ya limpia al reset, pero esto sirve para
 * refrescar durante la ejecucion. */
void vga_clear(void);

/* Limpia una sola fila 'y' (util para refrescar un dato sin
 * borrar toda la pantalla). */
void vga_clear_row(int y);

/* Escribe un entero sin signo 'value' en (x, y), en decimal.
 * Devuelve la cantidad de digitos escritos. */
int vga_print_uint(int x, int y, uint32_t value);

/* Escribe un tiempo en formato MM:SS en (x, y) a partir de
 * 'total_seconds'. Siempre ocupa 5 caracteres ("MM:SS"). */
void vga_print_time(int x, int y, uint32_t total_seconds);

#endif /* VGA_H */
