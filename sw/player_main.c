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
    "SONG4.WAV",
    "SONG5.WAV",
    "SONG6.WAV",
    "SONG7.WAV",
    "SONG8.WAV",
    "SONG9.WAV",
    "SONG10.WAV",
};
#define PLAYLIST_LEN (sizeof(PLAYLIST) / sizeof(PLAYLIST[0]))

/* Buffer de un bloque PCM (512 B = 128 palabras), alineado a 32 bits */
static uint32_t audio_block[128];

/* -------------------------------------------------------------------------
 * Reproduce una pista: abre, valida cabecera, anuncia al NIOS y hace streaming.
 * Devuelve 0 en éxito, negativo si no se pudo reproducir.
 * ------------------------------------------------------------------------- */
static uint8_t prefix[2048];     /* prefijo para escanear cabecera + metadatos */

static int reproducir_pista(const char* nombre) {
    Fat32File f;
    WavHeader wh;
    char     title[WAV_INFO_MAX];
    char     artist[WAV_INFO_MAX];
    uint32_t data_off = 0, data_size = 0;

    /* 1. Abrir y leer un prefijo (cabecera RIFF + chunk LIST/INFO + 'data') */
    if (fat32_open(nombre, &f) != 0) {
        printf("[WARN] No se encontro '%s', se omite.\n", nombre);
        return -1;
    }
    int pn = fat32_read(&f, prefix, sizeof(prefix));
    if (pn < 44) {
        printf("[WARN] '%s' demasiado corto.\n", nombre);
        return -1;
    }

    /* 2. Escanear chunks: formato + offset/tamano real de 'data' + titulo/artista.
     *    (ffmpeg mete un LIST/INFO antes de 'data', asi que NO esta en el offset 44.) */
    WavStatus st = wav_scan(prefix, (uint32_t)pn, &wh, &data_off, &data_size,
                            title, artist, WAV_INFO_MAX);
    if (st != WAV_OK) {
        printf("[WARN] '%s' no es WAV PCM valido (cod %d).\n", nombre, st);
        return -1;
    }

    printf("\n=== Reproduciendo %s ===\n", nombre);
    printf("Titulo : %s\n", title[0]  ? title  : "(sin titulo)");
    printf("Artista: %s\n", artist[0] ? artist : "(sin artista)");
    wav_print_info(&wh);

    /* 3. Anunciar la pista (metadatos numericos + texto) y dar PLAY */
    audio_bridge_init(&wh);                 /* CMD_TRACK_START + TrackMetadataPacket */
    audio_bridge_send_text(title, artist);  /* CMD_TRACK_TEXT  + titulo/artista      */
    audio_bridge_play();

    /* 4. Reposicionar al inicio del audio: reabrir y descartar 'data_off' bytes */
    fat32_open(nombre, &f);
    uint32_t to_skip = data_off;
    while (to_skip > 0) {
        uint32_t n = (to_skip > 512U) ? 512U : to_skip;
        int r = fat32_read(&f, audio_block, n);
        if (r <= 0) break;
        to_skip -= (uint32_t)r;
    }

    /* 5. Streaming del audio PCM en bloques de 512 B */
    uint32_t bloque = 0;
    uint32_t restantes = data_size;
    while (restantes > 0) {
        uint32_t pedir = (restantes >= 512U) ? 512U : restantes;
        memset(audio_block, 0, sizeof(audio_block));   /* silencio en el bloque final */
        int n = fat32_read(&f, audio_block, pedir);
        if (n <= 0) break;
        audio_bridge_send_block(audio_block, bloque++);
        restantes -= (uint32_t)n;
    }

    /* 6. Señalizar fin de pista */
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
