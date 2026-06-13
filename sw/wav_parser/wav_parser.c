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

/* --------------------------------------------------------------------------
 * Metadatos de texto (chunk LIST/INFO)
 * -------------------------------------------------------------------------- */

static uint32_t rd32le(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Copia n bytes de 'src' a 'dst' (con tope maxlen), nul-termina y recorta
 * espacios/nulos finales. */
static void copy_info_field(char* dst, uint32_t maxlen,
                            const uint8_t* src, uint32_t n) {
    uint32_t i;
    if (maxlen == 0) return;
    if (n > maxlen - 1) n = maxlen - 1;
    for (i = 0; i < n; i++) dst[i] = (char)src[i];
    dst[i] = '\0';
    while (i > 0) {
        char c = dst[i - 1];
        if (c == ' ' || c == '\0' || c == '\r' || c == '\n') {
            dst[--i] = '\0';
        } else {
            break;
        }
    }
}

int wav_parse_info(const uint8_t* buf, uint32_t len,
                   char* title, char* artist, uint32_t maxlen) {
    uint32_t pos;
    int found = 0;

    if (maxlen > 0) { title[0] = '\0'; artist[0] = '\0'; }
    if (len < 12) return 0;

    /* saltar "RIFF" + size(4) + "WAVE" */
    pos = 12;
    while (pos + 8 <= len) {
        const uint8_t* ck = buf + pos;
        uint32_t cksize = rd32le(ck + 4);

        /* ¿Es un chunk LIST de tipo INFO? */
        if (tag_equals(ck, "LIST") && pos + 12 <= len && tag_equals(ck + 8, "INFO")) {
            uint32_t sub = pos + 12;
            uint32_t end = pos + 8 + cksize;
            if (end > len) end = len;
            while (sub + 8 <= end) {
                const uint8_t* s = buf + sub;
                uint32_t ssize = rd32le(s + 4);
                if (sub + 8 + ssize > len) break;
                if (tag_equals(s, "INAM")) {
                    copy_info_field(title, maxlen, s + 8, ssize);
                    found++;
                } else if (tag_equals(s, "IART")) {
                    copy_info_field(artist, maxlen, s + 8, ssize);
                    found++;
                }
                sub += 8 + ssize + (ssize & 1);   /* siguiente subchunk (padding par) */
            }
            return found;
        }

        if (cksize == 0) break;                   /* proteccion contra bucle infinito */
        pos += 8 + cksize + (cksize & 1);         /* siguiente chunk (padding par) */
    }
    return found;
}

WavStatus wav_scan(const uint8_t* buf, uint32_t len, WavHeader* out,
                   uint32_t* data_offset, uint32_t* data_size,
                   char* title, char* artist, uint32_t txtmax) {
    uint32_t pos;
    int have_fmt = 0, have_data = 0;

    if (txtmax > 0) { title[0] = '\0'; artist[0] = '\0'; }
    *data_offset = 0;
    *data_size   = 0;

    if (len < 12)                    return WAV_ERR_NOT_RIFF;
    if (!tag_equals(buf, "RIFF"))    return WAV_ERR_NOT_RIFF;
    if (!tag_equals(buf + 8, "WAVE")) return WAV_ERR_NOT_WAVE;

    memset(out, 0, sizeof(*out));

    /* Recorrer chunks desde el offset 12 (tras "RIFF"+size+"WAVE") */
    pos = 12;
    while (pos + 8 <= len) {
        const uint8_t* ck = buf + pos;
        uint32_t cksize = rd32le(ck + 4);

        if (tag_equals(ck, "fmt ") && pos + 8 + 16 <= len) {
            const uint8_t* d = ck + 8;
            out->audio_format    = (uint16_t)(d[0]  | (d[1]  << 8));
            out->num_channels    = (uint16_t)(d[2]  | (d[3]  << 8));
            out->sample_rate     = rd32le(d + 4);
            out->byte_rate       = rd32le(d + 8);
            out->block_align     = (uint16_t)(d[12] | (d[13] << 8));
            out->bits_per_sample = (uint16_t)(d[14] | (d[15] << 8));
            have_fmt = 1;
        } else if (tag_equals(ck, "LIST") && pos + 12 <= len && tag_equals(ck + 8, "INFO")) {
            uint32_t sub = pos + 12;
            uint32_t end = pos + 8 + cksize;
            if (end > len) end = len;
            while (sub + 8 <= end) {
                const uint8_t* s = buf + sub;
                uint32_t ssize = rd32le(s + 4);
                if (sub + 8 + ssize > len) break;
                if (tag_equals(s, "INAM"))      copy_info_field(title,  txtmax, s + 8, ssize);
                else if (tag_equals(s, "IART")) copy_info_field(artist, txtmax, s + 8, ssize);
                sub += 8 + ssize + (ssize & 1);
            }
        } else if (tag_equals(ck, "data")) {
            *data_offset = pos + 8;     /* el audio empieza tras el id+size */
            *data_size   = cksize;
            have_data = 1;
            break;                      /* "data" marca el fin de la cabecera */
        }

        if (cksize == 0) break;
        pos += 8 + cksize + (cksize & 1);
    }

    if (!have_fmt)               return WAV_ERR_NO_DATA;
    if (out->audio_format != 1)  return WAV_ERR_NOT_PCM;
    if (!have_data)              return WAV_ERR_NO_DATA;

    /* Completar campos canonicos por compatibilidad con wav_print_info() */
    out->chunk_id[0]='R'; out->chunk_id[1]='I'; out->chunk_id[2]='F'; out->chunk_id[3]='F';
    out->format[0]='W';   out->format[1]='A';   out->format[2]='V';   out->format[3]='E';
    out->subchunk1_id[0]='f'; out->subchunk1_id[1]='m'; out->subchunk1_id[2]='t'; out->subchunk1_id[3]=' ';
    out->subchunk1_size = 16;
    out->subchunk2_id[0]='d'; out->subchunk2_id[1]='a'; out->subchunk2_id[2]='t'; out->subchunk2_id[3]='a';
    out->subchunk2_size = *data_size;
    return WAV_OK;
}
