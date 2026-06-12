/*
 * vga_ui.c
 *
 * Implementacion de la pantalla "Reproduciendo ahora" (ver vga_ui.h).
 * Usa unicamente el driver de caracteres de vga.h.
 *
 * R_SoC - REQ-09
 */
#include "vga_ui.h"
#include "vga.h"

/* --- Distribucion de la pantalla (grilla 80x60) --- */
#define UI_COL_LABEL   4     /* columna de las etiquetas            */
#define UI_COL_VALUE   15    /* columna donde empiezan los valores  */

#define ROW_HEADER     2
#define ROW_RULE       4
#define ROW_TITLE      9
#define ROW_ARTIST     11
#define ROW_DUR        14
#define ROW_TIME       16

/* Dibuja una linea horizontal de '=' en la fila y */
static void rule(int y)
{
    int x;
    for (x = 2; x < VGA_COLS - 2; x++) {
        vga_putchar(x, y, '=');
    }
}

void vga_ui_init(void)
{
    vga_clear();
    vga_print_centered(ROW_HEADER, "REPRODUCTOR DE AUDIO  -  CE1113");
    rule(ROW_RULE);

    /* Etiquetas fijas */
    vga_print(UI_COL_LABEL, ROW_TITLE,  "Titulo  :");
    vga_print(UI_COL_LABEL, ROW_ARTIST, "Artista :");
    vga_print(UI_COL_LABEL, ROW_DUR,    "Duracion:");
    vga_print(UI_COL_LABEL, ROW_TIME,   "Tiempo  :");
}

/* Imprime el valor de un campo de texto, limpiando primero la zona de valor */
static void set_text_field(int row, const char *value)
{
    int x;
    /* limpiar solo la zona del valor (no la etiqueta) */
    for (x = UI_COL_VALUE; x < VGA_COLS - 1; x++) {
        vga_putchar(x, row, ' ');
    }
    vga_print(UI_COL_VALUE, row, value);
}

void vga_ui_set_track(const TrackInfo *t)
{
    set_text_field(ROW_TITLE,  t->title);
    set_text_field(ROW_ARTIST, t->artist);

    /* Duracion total en MM:SS */
    set_text_field(ROW_DUR, "");
    vga_print_time(UI_COL_VALUE, ROW_DUR, t->duration_sec);

    /* Reiniciar el tiempo transcurrido a 00:00 */
    vga_ui_set_elapsed(0);
}

void vga_ui_set_elapsed(uint32_t elapsed_sec)
{
    set_text_field(ROW_TIME, "");
    vga_print_time(UI_COL_VALUE, ROW_TIME, elapsed_sec);
}
