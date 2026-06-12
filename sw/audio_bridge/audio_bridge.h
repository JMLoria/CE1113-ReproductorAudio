#ifndef AUDIO_BRIDGE_H
#define AUDIO_BRIDGE_H

#include <stdint.h>
#include "wav_parser.h"

/* ==========================================================================
 *
 * El HPS inyecta una palabra de control ANTES de los datos para que el NIOS
 * sepa qué tipo de payload sigue. Los 8 bits altos identifican el comando;
 * los 24 bits bajos transportan datos auxiliares cuando aplica.
 *
 *  [31:24]  Opcode del comando
 *  [23:0]   Payload (depende del comando, ver tabla abajo)
 *
 *  CMD_TRACK_START  (0xA0): inicia pista nueva.
 *                           Payload [23:0] = total de bloques de 512 B a enviar.
 *                           Inmediatamente después el HPS envía un TrackMetadataPacket
 *                           (5 palabras de 32 bits).
 *
 *  CMD_BLOCK_READY  (0xB0): bloque de 128 palabras PCM listo en el FIFO.
 *                           Payload [23:0] = índice del bloque (0-based).
 *
 *  CMD_TRACK_END    (0xC0): fin de la pista, no vienen más bloques.
 *                           Payload = 0.
 *
 *  CMD_PLAY         (0xD0): orden de inicio/reanudación de reproducción.
 *  CMD_PAUSE        (0xE0): orden de pausa (el NIOS congela su máquina de estados).
 * ========================================================================== */
#define CMD_OPCODE_MASK     0xFF000000U

#define CMD_TRACK_START     0xA0000000U
#define CMD_BLOCK_READY     0xB0000000U
#define CMD_TRACK_END       0xC0000000U
#define CMD_PLAY            0xD0000000U
#define CMD_PAUSE           0xE0000000U
#define CMD_TRACK_TEXT      0xF0000000U   /* metadatos de texto (titulo + artista) */

/* Macros para construir / decodificar palabras de control */
#define CMD_BUILD(opcode, payload)  ((opcode) | ((payload) & 0x00FFFFFFU))
#define CMD_OPCODE(word)            ((word) & CMD_OPCODE_MASK)
#define CMD_PAYLOAD(word)           ((word) & 0x00FFFFFFU)

/* ==========================================================================
 * Estructura de metadatos de la pista (TrackMetadataPacket)
 *
 * Se transmite como 5 palabras de 32 bits consecutivas en el FIFO,
 * inmediatamente después de la palabra CMD_TRACK_START.
 *
 * Palabra 0 → magic      (siempre TRACK_META_MAGIC, sirve de verificación)
 * Palabra 1 → sample_rate
 * Palabra 2 → [31:16] num_channels  |  [15:0] bits_per_sample
 * Palabra 3 → total_audio_bytes (subchunk2_size del WAV)
 * Palabra 4 → total_blocks      (cuántos bloques de 512 B enviará el HPS)
 * ========================================================================== */
#define TRACK_META_MAGIC    0xAD100001U   /* Firma para validación en el NIOS */
#define TRACK_META_WORDS    5             /* Número de uint32_t que ocupa el paquete */

typedef struct __attribute__((packed)) {
    uint32_t magic;             /* TRACK_META_MAGIC                              */
    uint32_t sample_rate;       /* Hz  (ej. 44100)                               */
    uint32_t channels_and_bits; /* [31:16] = num_channels | [15:0] bits_per_sample */
    uint32_t total_audio_bytes; /* WavHeader.subchunk2_size                      */
    uint32_t total_blocks;      /* ceil(total_audio_bytes / 512)                 */
} TrackMetadataPacket;

/* Helper para construir el campo empaquetado channels_and_bits */
#define PACK_CHAN_BITS(ch, bps)  (((uint32_t)(ch) << 16) | ((uint32_t)(bps) & 0xFFFFU))

/* ==========================================================================
 * API pública
 * ========================================================================== */

/**
 * @brief Transmite los metadatos de la pista al NIOS y señaliza inicio de stream.
 *
 * Envía al FIFO (en orden):
 *   1. Palabra CMD_TRACK_START con total_blocks en el payload.
 *   2. Las 5 palabras del TrackMetadataPacket.
 *   3. Palabra CMD_PLAY para que el NIOS arranque su reproductor.
 *
 * Debe llamarse UNA VEZ por pista, antes de cualquier audio_bridge_send_block().
 *
 * @param header  Puntero al WavHeader validado por wav_parse_header().
 */
void audio_bridge_init(const WavHeader* header);

/**
 * @brief Envía 512 bytes (128 words de 32 bits) de audio PCM al NIOS.
 *
 * Precede los datos con una palabra CMD_BLOCK_READY que incluye el índice
 * del bloque, para que el NIOS pueda detectar paquetes perdidos o desfasados.
 *
 * Bloquea por polling si el FIFO está lleno (backpressure de hardware).
 *
 * @param buffer_512b  Puntero a las 128 palabras leídas desde la SD.
 * @param block_index  Índice 0-based del bloque actual (para CMD_BLOCK_READY).
 */
void audio_bridge_send_block(const uint32_t* buffer_512b, uint32_t block_index);

/**
 * @brief Señaliza al NIOS que la pista terminó (no vendrán más bloques).
 * Envía CMD_TRACK_END al FIFO.
 */
void audio_bridge_track_end(void);

/**
 * @brief Envía CMD_PLAY al NIOS (inicio o reanudación de reproducción).
 */
void audio_bridge_play(void);

/**
 * @brief Envía CMD_PAUSE al NIOS (congela la máquina de estados de reproducción).
 */
void audio_bridge_pause(void);

/* ==========================================================================
 * Paquete de texto (CMD_TRACK_TEXT)
 *
 * Después de la palabra CMD_TRACK_TEXT (con payload = TRACK_TEXT_WORDS) se
 * envían DOS campos de texto de longitud fija, cada uno de
 * TRACK_TEXT_FIELD_WORDS palabras (TRACK_TEXT_FIELD_BYTES bytes), en orden
 * little-endian y rellenados con ceros:
 *     1) titulo   (INAM del WAV)
 *     2) artista  (IART del WAV)
 *
 * Decodificación en el NIOS: al ver CMD_TRACK_TEXT, leer TRACK_TEXT_WORDS
 * palabras; las primeras TRACK_TEXT_FIELD_WORDS son el titulo y las siguientes
 * el artista. Cada palabra trae 4 bytes (byte 0 en los bits [7:0], etc.).
 * Como van rellenados con ceros, cada campo queda nul-terminado.
 * ========================================================================== */
#define TRACK_TEXT_FIELD_BYTES  48
#define TRACK_TEXT_FIELD_WORDS  12               /* 48 / 4 */
#define TRACK_TEXT_WORDS        (2 * TRACK_TEXT_FIELD_WORDS)

/**
 * @brief Envía los metadatos de texto (titulo + artista) al NIOS.
 * Llamar una vez por pista, junto con audio_bridge_init().
 */
void audio_bridge_send_text(const char* title, const char* artist);

#endif /* AUDIO_BRIDGE_H */