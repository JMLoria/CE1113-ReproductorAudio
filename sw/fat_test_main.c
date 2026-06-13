#include "sd_driver.h"
#include "fat32.h"
#include "wav_parser.h"
#include <stdio.h>

/* =============================================================================
 * Prueba bare-metal del lector FAT32:
 *   1. Inicializa la SD.
 *   2. Monta el sistema de archivos.
 *   3. Lista el directorio raíz.
 *   4. Abre una canción por nombre, lee su cabecera y la parsea como WAV.
 *
 * Cambiá TRACK_NAME por el nombre real de tu archivo en la SD (8.3).
 * ========================================================================== */
#define TRACK_NAME "SONG1.WAV"

int main(void) {
    printf("\n=== PRUEBA LECTOR FAT32 (HPS BARE-METAL) ===\n");

    sd_init();

    int r = fat32_mount();
    if (r != 0) {
        printf("[ERROR] fat32_mount fallo (codigo %d)\n", r);
        return 1;
    }
    printf("[OK] Volumen FAT32 montado.\n\n");

    fat32_list_root();

    Fat32File f;
    r = fat32_open(TRACK_NAME, &f);
    if (r != 0) {
        printf("\n[ERROR] No se encontro '%s' (codigo %d)\n", TRACK_NAME, r);
        return 1;
    }
    printf("\n[OK] '%s' abierto: %u bytes.\n", TRACK_NAME, (unsigned)f.size);

    /* Leer los primeros 44 bytes (cabecera WAV) */
    uint8_t header[WAV_HEADER_SIZE];
    int n = fat32_read(&f, header, sizeof(header));
    printf("[INFO] Leidos %d bytes de cabecera.\n", n);

    WavHeader h;
    WavStatus st = wav_parse_header(header, &h);
    if (st == WAV_OK) {
        printf("[OK] WAV valido:\n");
        wav_print_info(&h);
    } else {
        printf("[ERROR] WAV invalido (codigo %d)\n", st);
    }

    return 0;
}
