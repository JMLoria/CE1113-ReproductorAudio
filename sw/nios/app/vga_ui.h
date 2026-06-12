/*
 * vga_ui.h
 *
 * Pantalla "Reproduciendo ahora" para la VGA (REQ-09).
 * Compone los metadatos de la pista sobre el driver de caracteres (vga.h):
 * titulo, artista, duracion total y tiempo transcurrido (MM:SS).
 *
 * R_SoC - REQ-09
 */
#ifndef VGA_UI_H
#define VGA_UI_H

#include <stdint.h>

/* Longitud maxima de cada campo de texto (cabe en una fila de 80 chars
 * dejando espacio para la etiqueta). Incluye el '\0'. */
#define UI_TEXT_MAX 48

/* Metadatos de la pista a mostrar. Los llena el software de control a partir
 * de lo que el HPS envia por el FIFO (ver protocolo audio_bridge). */
typedef struct {
    char     title[UI_TEXT_MAX];
    char     artist[UI_TEXT_MAX];
    uint32_t duration_sec;   /* duracion total en segundos */
} TrackInfo;

/* Dibuja el marco fijo de la pantalla (titulo del programa y etiquetas).
 * Llamar una vez al inicio (o tras un cambio de modo de pantalla). */
void vga_ui_init(void);

/* Actualiza los campos de la pista (titulo/artista/duracion).
 * Llamar al empezar cada cancion. */
void vga_ui_set_track(const TrackInfo *t);

/* Refresca el tiempo transcurrido (MM:SS). Llamar cada segundo desde el
 * lazo principal o la ISR del timer. */
void vga_ui_set_elapsed(uint32_t elapsed_sec);

#endif /* VGA_UI_H */
