/*
 * main.c  -  Prueba "hello world" del driver VGA (REQ-09)
 *
 * Objetivo: confirmar que la cadena
 *     NIOS II -> char_buffer -> vga_controller -> monitor
 * funciona de extremo a extremo. Escribe texto fijo en pantalla
 * y ejercita las primitivas del driver (vga.h / vga.c).
 *
 * Si el texto aparece en el monitor VGA, la base de REQ-09 esta
 * lista y se puede pasar a integrar la UI con metadatos.
 *
 * Proyecto: CE1113-ReproductorAudio (Grupo 2 - TEC)
 * Rol: R_SoC
 */

#include "vga.h"
#include "sys/alt_stdio.h"

int main(void)
{
    /* Mensaje por JTAG UART: confirma que el NIOS arranco y corre
     * codigo antes de tocar el hardware de video. Util si la pantalla
     * queda en blanco: si ves esto en la consola pero no en el monitor,
     * el problema esta en el char_buffer / vga_controller, no en el NIOS. */
    alt_putstr("Prueba VGA - REQ-09\n");

    /* Limpiar la pantalla. El hardware ya limpia al reset, pero lo
     * hacemos explicito para no depender de ello. */
    vga_clear();

    /* Titulos centrados horizontalmente */
    vga_print_centered(5, "REPRODUCTOR DE AUDIO - CE1113");
    vga_print_centered(7, "Grupo 2 - TEC");

    /* Texto de prueba en posiciones fijas */
    vga_print(10, 12, "Hola mundo desde NIOS II!");
    vga_print(10, 14, "Driver VGA funcionando.");

    /* Demostrar el formato de tiempo MM:SS (125 s = 02:05) */
    vga_print(10, 18, "Tiempo de ejemplo:");
    vga_print_time(29, 18, 125);

    /* Demostrar impresion de enteros sin signo */
    vga_print(10, 20, "Numero de ejemplo:");
    vga_print_uint(29, 20, 48000);

    /* Marcar las 4 esquinas para verificar los bordes de la grilla 80x60.
     * Si las cuatro '+' se ven en las esquinas, el mapeo (x,y) es correcto. */
    vga_putchar(0, 0, '+');
    vga_putchar(VGA_COLS - 1, 0, '+');
    vga_putchar(0, VGA_ROWS - 1, '+');
    vga_putchar(VGA_COLS - 1, VGA_ROWS - 1, '+');

    alt_putstr("Texto escrito en pantalla VGA.\n");

    /* No hay mas que hacer: el char_buffer retiene el texto escrito.
     * Bucle infinito para mantener la imagen en pantalla. */
    while (1) {
        /* idle */
    }

    return 0;
}
