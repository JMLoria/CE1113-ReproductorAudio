#ifndef WAV_PARSER_H
#define WAV_PARSER_H

#include <stdint.h>

/**
 * @brief Encabezado canónico WAV PCM de 44 bytes.
 * __attribute__((packed)) es crítico: sin él, el compilador ARM inserta
 * 2 bytes de padding entre num_channels (uint16_t) y sample_rate (uint32_t),
 * corrompiendo todos los campos que siguen.
 */
typedef struct __attribute__((packed)) {
    /* --- Chunk RIFF --- */
    uint8_t  chunk_id[4];       /* "RIFF"                                  */
    uint32_t chunk_size;        /* Tamaño total del archivo - 8 bytes      */
    uint8_t  format[4];         /* "WAVE"                                  */

    /* --- Sub-chunk "fmt " --- */
    uint8_t  subchunk1_id[4];   /* "fmt "                                  */
    uint32_t subchunk1_size;    /* 16 para PCM lineal                      */
    uint16_t audio_format;      /* 1 = PCM, cualquier otro = comprimido    */
    uint16_t num_channels;      /* 1 = mono, 2 = estéreo                   */
    uint32_t sample_rate;       /* Muestras por segundo (Hz)               */
    uint32_t byte_rate;         /* SampleRate × NumChannels × BitsPerSample/8 */
    uint16_t block_align;       /* NumChannels × BitsPerSample/8           */
    uint16_t bits_per_sample;   /* 8, 16 o 32                              */

    /* --- Sub-chunk "data" --- */
    uint8_t  subchunk2_id[4];   /* "data"                                  */
    uint32_t subchunk2_size;    /* Bytes de audio puro que siguen          */

} WavHeader;

/* Tamaño esperado: debe ser exactamente 44 bytes */
#define WAV_HEADER_SIZE 44

/* Offset desde el inicio del buffer donde comienzan las muestras PCM */
#define WAV_PCM_OFFSET  sizeof(WavHeader)

/**
 * @brief Códigos de resultado del parser.
 */
typedef enum {
    WAV_OK              = 0,  /* Header válido, out_header fue llenado     */
    WAV_ERR_NOT_RIFF    = 1,  /* Los primeros 4 bytes no son "RIFF"        */
    WAV_ERR_NOT_WAVE    = 2,  /* Los bytes 8-11 no son "WAVE"              */
    WAV_ERR_NOT_PCM     = 3,  /* audio_format != 1 (comprimido/extensible) */
    WAV_ERR_NO_DATA     = 4,  /* El sub-chunk "data" no está en offset 36  */
} WavStatus;

/**
 * @brief Parsea los primeros 44 bytes del buffer como un header WAV PCM.
 * Opera por cast directo — sin copias intermedias.
 * @param buffer  Puntero al buffer leído desde la SD (al menos 44 bytes).
 * @param out_header  Puntero donde se copia el header validado.
 * @return WAV_OK si el archivo es PCM estándar, o un código de error.
 */
WavStatus wav_parse_header(const uint8_t* buffer, WavHeader* out_header);

/**
 * @brief Imprime via semihosting los metadatos del header.
 * @param header  Puntero a un WavHeader previamente validado.
 */
void wav_print_info(const WavHeader* header);

/* Longitud recomendada (con '\0' incluido) para los campos de metadatos. */
#define WAV_INFO_MAX 48

/**
 * @brief Extrae titulo (INAM) y artista (IART) de un chunk LIST/INFO que este
 * presente dentro de los primeros 'len' bytes de 'buf'. Si no hay metadatos,
 * deja los strings vacios. Opera solo sobre el buffer (sin acceso a archivo).
 * @param buf     Prefijo del archivo WAV (idealmente cabecera + chunk LIST).
 * @param len     Cantidad de bytes validos en 'buf'.
 * @param title   Buffer de salida para el titulo (al menos 'maxlen' bytes).
 * @param artist  Buffer de salida para el artista (al menos 'maxlen' bytes).
 * @param maxlen  Tamano de cada buffer de salida (incluye el '\0').
 * @return Cantidad de campos encontrados (0..2).
 */
int wav_parse_info(const uint8_t* buf, uint32_t len,
                   char* title, char* artist, uint32_t maxlen);

/**
 * @brief Escanea la cabecera RIFF tolerando chunks intermedios (p.ej. un
 * LIST/INFO antes de "data", como genera ffmpeg). Encuentra el formato, el
 * offset y tamano reales del audio, y extrae titulo/artista si hay LIST/INFO.
 *
 * A diferencia de wav_parse_header (que asume el layout canonico con "data"
 * en el offset 36), esto funciona aunque "data" este mas adelante.
 *
 * @param buf          Prefijo del archivo (debe llegar al menos hasta "data").
 * @param len          Bytes validos en 'buf'.
 * @param out          Header con el formato (sample_rate, canales, bits, etc.).
 * @param data_offset  [out] offset en bytes donde empieza el audio PCM.
 * @param data_size    [out] cantidad de bytes de audio PCM.
 * @param title        [out] titulo (INAM) o cadena vacia.
 * @param artist       [out] artista (IART) o cadena vacia.
 * @param txtmax       Tamano de los buffers title/artist.
 * @return WAV_OK si encontro fmt + data PCM; un codigo de error si no.
 */
WavStatus wav_scan(const uint8_t* buf, uint32_t len, WavHeader* out,
                   uint32_t* data_offset, uint32_t* data_size,
                   char* title, char* artist, uint32_t txtmax);

#endif /* WAV_PARSER_H */
