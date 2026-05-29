#include "wav_parser.h"
#include <stdio.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Helpers internos
 * -------------------------------------------------------------------------- */

/**
 * Compara 4 bytes de un campo del header contra un literal de 4 caracteres.
 * No usa strcmp porque los campos NO están nul-terminados en el formato WAV.
 */
static int tag_equals(const uint8_t* field, const char* tag) {
    return (field[0] == (uint8_t)tag[0] &&
            field[1] == (uint8_t)tag[1] &&
            field[2] == (uint8_t)tag[2] &&
            field[3] == (uint8_t)tag[3]);
}

/* --------------------------------------------------------------------------
 * API pública
 * -------------------------------------------------------------------------- */

WavStatus wav_parse_header(const uint8_t* buffer, WavHeader* out_header) {
    /*
     * Cast directo al struct — cero copias, cero malloc.
     * Funciona porque __attribute__((packed)) garantiza que el layout del
     * struct coincide byte a byte con el formato del archivo.
     */
    const WavHeader* h = (const WavHeader*) buffer;

    /* Verificar firma RIFF */
    if (!tag_equals(h->chunk_id, "RIFF")) {
        return WAV_ERR_NOT_RIFF;
    }

    /* Verificar que es WAVE y no otro formato RIFF (AVI, ANI, etc.) */
    if (!tag_equals(h->format, "WAVE")) {
        return WAV_ERR_NOT_WAVE;
    }

    /* Solo soportamos PCM lineal (audio_format == 1).
     * Valor 3 = IEEE float, 6 = A-law, 7 = mu-law, 0xFFFE = extensible. */
    if (h->audio_format != 1) {
        return WAV_ERR_NOT_PCM;
    }

    /* Verificar que el sub-chunk de datos está en la posición canónica */
    if (!tag_equals(h->subchunk2_id, "data")) {
        return WAV_ERR_NO_DATA;
    }

    /* Header válido: copiar al out para que el caller lo use */
    *out_header = *h;
    return WAV_OK;
}

void wav_print_info(const WavHeader* h) {
    uint32_t duration_ms = 0;
    if (h->byte_rate > 0) {
        duration_ms = (h->subchunk2_size * 1000u) / h->byte_rate;
    }

    printf("\n=== METADATOS WAV ===\n");
    printf("Canales          : %u (%s)\n",
           h->num_channels,
           h->num_channels == 1 ? "Mono" :
           h->num_channels == 2 ? "Estéreo" : "Multicanal");
    printf("Sample Rate      : %lu Hz\n",  (unsigned long)h->sample_rate);
    printf("Bits por muestra : %u bits\n", h->bits_per_sample);
    printf("Byte Rate        : %lu B/s\n", (unsigned long)h->byte_rate);
    printf("Block Align      : %u bytes\n", h->block_align);
    printf("Datos de audio   : %lu bytes\n", (unsigned long)h->subchunk2_size);
    printf("Duración aprox.  : %lu ms (%lu s)\n",
           (unsigned long)duration_ms,
           (unsigned long)(duration_ms / 1000u));
    printf("====================\n");
}
