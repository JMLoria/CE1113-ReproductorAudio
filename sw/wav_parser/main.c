#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "wav_parser.h"

/* -------------------------------------------------------------------------
 * WAV PCM mínimo de 44 bytes embebido como literal
 *
 * Parámetros del audio ficticio:
 *   - 1 canal (Mono)
 *   - 8000 Hz sample rate
 *   - 16 bits por muestra
 *   - 0 bytes de datos de audio (solo testemos el header)
 *
 * Todos los campos en little-endian, que es el byte order del formato WAV
 * y también el del Cortex-A9 en modo ARM.
 * -------------------------------------------------------------------------*/
static const uint8_t valid_wav[WAV_HEADER_SIZE] = {
    /* chunk_id       */ 'R', 'I', 'F', 'F',
    /* chunk_size     */ 36, 0, 0, 0,          /* 44 - 8 = 36 (sin datos)  */
    /* format         */ 'W', 'A', 'V', 'E',

    /* subchunk1_id   */ 'f', 'm', 't', ' ',
    /* subchunk1_size */ 16, 0, 0, 0,           /* PCM lineal = 16 bytes    */
    /* audio_format   */ 0x01, 0x00,            /* 1 = PCM                  */
    /* num_channels   */ 0x01, 0x00,            /* 1 = Mono                 */
    /* sample_rate    */ 0x40, 0x1F, 0x00, 0x00,/* 8000 Hz (0x00001F40)     */
    /* byte_rate      */ 0x80, 0x3E, 0x00, 0x00,/* 16000 B/s (8000×1×2)    */
    /* block_align    */ 0x02, 0x00,            /* 2 bytes (1 ch × 16 bit)  */
    /* bits_per_sample*/ 0x10, 0x00,            /* 16 bits                  */

    /* subchunk2_id   */ 'd', 'a', 't', 'a',
    /* subchunk2_size */ 0x00, 0x00, 0x00, 0x00 /* 0 bytes de audio         */
};

/* Header con magic RIFF corrupto */
static const uint8_t bad_riff[WAV_HEADER_SIZE] = {
    'X', 'X', 'X', 'X',   /* chunk_id incorrecto */
    36, 0, 0, 0,
    'W', 'A', 'V', 'E',
    'f', 'm', 't', ' ',
    16, 0, 0, 0,
    0x01, 0x00,
    0x01, 0x00,
    0x40, 0x1F, 0x00, 0x00,
    0x80, 0x3E, 0x00, 0x00,
    0x02, 0x00,
    0x10, 0x00,
    'd', 'a', 't', 'a',
    0x00, 0x00, 0x00, 0x00
};

/* Header PCM con audio_format = 0x0055 (MPEG / MP3) */
static const uint8_t compressed_wav[WAV_HEADER_SIZE] = {
    'R', 'I', 'F', 'F',
    36, 0, 0, 0,
    'W', 'A', 'V', 'E',
    'f', 'm', 't', ' ',
    16, 0, 0, 0,
    0x55, 0x00,            /* audio_format = 85 (0x55) = MPEG Layer III    */
    0x02, 0x00,
    0x44, 0xAC, 0x00, 0x00,/* 44100 Hz */
    0x00, 0x7D, 0x00, 0x00,
    0x04, 0x00,
    0x10, 0x00,
    'd', 'a', 't', 'a',
    0x00, 0x00, 0x00, 0x00
};

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

static void run_test(const char* name,
                     const uint8_t* buffer,
                     WavStatus expected)
{
    WavHeader hdr;
    memset(&hdr, 0, sizeof(hdr));

    WavStatus got = wav_parse_header(buffer, &hdr);

    tests_run++;
    if (got == expected) {
        tests_passed++;
        printf("[PASS] %s\n", name);
        if (got == WAV_OK) {
            wav_print_info(&hdr);
        }
    } else {
        tests_failed++;
        printf("[FAIL] %s  (esperado=%d, obtenido=%d)\n",
               name, (int)expected, (int)got);
    }
}

static void check_struct_size(void) {
    printf("\n--- Verificación de layout ---\n");
    printf("sizeof(WavHeader) = %u bytes (esperado: %u)\n",
           (unsigned)sizeof(WavHeader),
           (unsigned)WAV_HEADER_SIZE);

    if (sizeof(WavHeader) == WAV_HEADER_SIZE) {
        printf("[PASS] Layout struct correcto — __attribute__((packed)) OK\n");
        tests_passed++;
    } else {
        printf("[FAIL] Layout incorrecto — probable padding sin packed\n");
        tests_failed++;
    }
    tests_run++;
}

/* -------------------------------------------------------------------------
 * Entry point
 * -------------------------------------------------------------------------*/
int main(void) {
    printf("\n======================================\n");
    printf("  wav_parser test — QEMU versatilepb  \n");
    printf("======================================\n");

    /* 1. Verificar que el struct mide exactamente 44 bytes */
    check_struct_size();

    /* 2. Caso nominal: WAV PCM válido */
    run_test("WAV PCM válido (8kHz, 16b, mono)",
             valid_wav, WAV_OK);

    /* 3. Magic RIFF incorrecto */
    run_test("Magic RIFF incorrecto",
             bad_riff, WAV_ERR_NOT_RIFF);

    /* 4. Formato comprimido (audio_format != 1) */
    run_test("Formato comprimido (MPEG, audio_format=0x55)",
             compressed_wav, WAV_ERR_NOT_PCM);

    /* ---- Resumen ---- */
    printf("\n======================================\n");
    printf("  Resultados: %d/%d tests pasados\n", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf("  FALLOS: %d\n", tests_failed);
    }
    printf("======================================\n\n");

    return (tests_failed == 0) ? 0 : 1;
}