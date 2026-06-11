/*
 * main.c  -  Prueba "hello world" del driver VGA (REQ-09)
 *
 * Objetivo: confirmar que la cadena NIOS -> char_buffer ->
 * vga_controller -> monitor funciona. Escribe texto fijo en
 * pantalla y demuestra las primitivas del driver.
 *
 * Si se ve el texto en el monitor VGA, la base de REQ-09 esta lista.
 *
 * R_SoC
 */
#include "vga.h"
#include "sys/alt_stdio.h"

int main(void)
{
    alt_putstr("Prueba VGA - REQ-09\n");

    /* Limpiar la pantalla (el HW ya lo hace al reset, pero por las dudas) */
    vga_clear();

    /* Titulo centrado */
    vga_print_centered(5, "REPRODUCTOR DE AUDIO - CE1113");
    vga_print_centered(7, "Grupo 2 - TEC");

    /* Texto de prueba en posiciones fijas */
    vga_print(10, 12, "Hola mundo desde NIOS II!");
    vga_print(10, 14, "Driver VGA funcionando.");

    /* Demostrar el formato de tiempo (02:05) */
    vga_print(10, 18, "Tiempo de ejemplo:");
    vga_print_time(29, 18, 125);   /* 125 s = 02:05 */

    /* Demostrar impresion de numeros */
    vga_print(10, 20, "Numero de ejemplo:");
    vga_print_uint(29, 20, 48000);

    /* Marco simple en las esquinas para verificar los bordes 80x60 */
    vga_putchar(0, 0, '+');
    vga_putchar(VGA_COLS - 1, 0, '+');
    vga_putchar(0, VGA_ROWS - 1, '+');
    vga_putchar(VGA_COLS - 1, VGA_ROWS - 1, '+');

    alt_putstr("Texto escrito en pantalla VGA.\n");

    /* No hay nada mas que hacer: el char_buffer retiene el texto. */
    while (1) {
        /* bucle infinito; la pantalla mantiene lo escrito */
    }

    return 0;
}
