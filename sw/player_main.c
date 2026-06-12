#include "sd_driver.h"
#include "fat32.h"
#include "wav_parser.h"
#include "audio_bridge.h"
#include <stdio.h>
#include <string.h>

/* =============================================================================
 * Reproductor de audio — aplicación BARE-METAL del HPS (ARM Cortex-A9).
 *
 * Flujo completo del lado del HPS:
 *   SD (sd_driver) -> FAT32 (fat32) -> WAV (wav_parser) -> FIFO IPC (audio_bridge)
 * El NIOS II (en la FPGA) consume el FIFO, aplica el filtro seleccionado por los
 * switches y envía el audio al códec WM8731.
 *
 * Las canciones se leen por NOMBRE desde la raíz de la SD (FAT32, nombres 8.3).
 * Editá PLAYLIST con los nombres reales de tus archivos.
 * ========================================================================== */

static const char* PLAYLIST[] = {
    "SONG1.WAV",
    "SONG2.WAV",
    "SONG3.WAV",
};
#define PLAYLIST_LEN (sizeof(PLAYLIST) / sizeof(PLAYLIST[0]))

/* Buffer de un bloque PCM (512 B = 128 palabras), alineado a 32 bits */
static uint32_t audio_block[128];

/* -------------------------------------------------------------------------
 * Reproduce una pista: abre, valida cabecera, anuncia al NIOS y hace streaming.
 * Devuelve 0 en éxito, negativo si no se pudo reproducir.
 * ------------------------------------------------------------------------- */
static int reproducir_pista(const char* nombre) {
    Fat32File f;
    if (fat32_open(nombre, &f) != 0) {
        printf("[WARN] No se encontro '%s', se omite.\n", nombre);
        return -1;
    }

    /* 1. Leer y validar la cabecera WAV (44 bytes) */
    uint8_t header[WAV_HEADER_SIZE];
    if (fat32_read(&f, header, WAV_HEADER_SIZE) != (int)WAV_HEADER_SIZE) {
        printf("[WARN] '%s' demasiado corto.\n", nombre);
        return -1;
    }
    WavHeader wh;
    if (wav_parse_header(header, &wh) != WAV_OK) {
        printf("[WARN] '%s' no es WAV PCM valido.\n", nombre);
        return -1;
    }

    printf("\n=== Reproduciendo %s ===\n", nombre);
    wav_print_info(&wh);

    /* 2. Anunciar la pista al NIOS (CMD_TRACK_START + metadatos) y dar PLAY */
    audio_bridge_init(&wh);
    audio_bridge_play();

    /* 3. Streaming del audio PCM en bloques de 512 B.
     *    Leemos solo subchunk2_size bytes (el audio real, ya saltada la cabecera). */
    uint32_t bloque    = 0;
    uint32_t restantes = wh.subchunk2_size;
    while (restantes > 0) {
        uint32_t pedir = (restantes >= 512U) ? 512U : restantes;

        /* Padding de silencio para el bloque fraccionario final */
        memset(audio_block, 0, sizeof(audio_block));

        int n = fat32_read(&f, audio_block, pedir);
        if (n <= 0) break;                 /* EOF inesperado */

        audio_bridge_send_block(audio_block, bloque++);
        restantes -= (uint32_t)n;
    }

    /* 4. Señalizar fin de pista */
    audio_bridge_track_end();
    printf("[OK] %s finalizada (%lu bloques).\n", nombre, (unsigned long)bloque);
    return 0;
}

int main(void) {
    printf("\n=== REPRODUCTOR DE AUDIO (HPS BARE-METAL) ===\n");

    /* 1. Inicializar la SD y montar el sistema de archivos */
    sd_init();
    if (fat32_mount() != 0) {
        printf("[ERROR] No se pudo montar la SD (FAT32). Abortando.\n");
        return 1;
    }
    printf("[OK] SD montada.\n");
    fat32_list_root();

    /* 2. Reproducción secuencial de la lista (REQ-02) */
    for (uint32_t i = 0; i < PLAYLIST_LEN; i++) {
        reproducir_pista(PLAYLIST[i]);
    }

    printf("\n[FIN] Lista de reproduccion completa.\n");
    return 0;
}
