#include "audio_bridge.h"
#include <stdio.h>

#define HPS_LW_BRIDGE_BASE     0xFF200000U
#define FIFO_IN_DATA_ADDR      (HPS_LW_BRIDGE_BASE + 0x00000U)   /* fifo.in      W  */
#define FIFO_IN_CSR_ADDR       (HPS_LW_BRIDGE_BASE + 0xA0020U)   /* fifo.in_csr  R/W */

/* Offsets dentro del bloque CSR (en bytes) */
#define CSR_LEVEL_OFFSET       0x00U   /* Número de palabras en el FIFO             */
#define CSR_STATUS_OFFSET      0x04U   /* Flags de estado                           */

/* Máscaras del registro STATUS (ipc_fifo.md) */
#define FIFO_STATUS_FULL       0x01U   /* Bit 0: FIFO lleno                         */
#define FIFO_STATUS_EMPTY      0x02U   /* Bit 1: FIFO vacío                         */
#define FIFO_STATUS_OVF        0x10U   /* Bit 4: overflow                           */

#ifdef QEMU_TEST
    static uint32_t mock_fifo_data   = 0;
    static uint32_t mock_csr_level   = 0;
    static uint32_t mock_csr_status  = 0;   /* 0 = no lleno, listo para escribir */

    /* Acceso al registro de datos y al bloque CSR simulado */
    #define FIFO_DATA_REG                    (&mock_fifo_data)
    #define CSR_REG(offset_bytes)            ((volatile uint32_t*)( \
                                               (offset_bytes) == CSR_LEVEL_OFFSET  ? (uintptr_t)&mock_csr_level  : \
                                                                                     (uintptr_t)&mock_csr_status ))
#else
    #define FIFO_DATA_REG                    ((volatile uint32_t*) FIFO_IN_DATA_ADDR)
    #define CSR_REG(offset_bytes)            ((volatile uint32_t*)(FIFO_IN_CSR_ADDR + (offset_bytes)))
#endif

static void fifo_write_word(uint32_t word) {
    /* Esperar mientras el FIFO esté lleno */
    while (*CSR_REG(CSR_STATUS_OFFSET) & FIFO_STATUS_FULL) {
#ifdef QEMU_TEST
        /* En simulación el consumidor es ficticio: forzamos bit a 0 y salimos */
        *CSR_REG(CSR_STATUS_OFFSET) &= ~FIFO_STATUS_FULL;
        break;
#endif
        /* Hardware real: el NIOS consume, el hardware baja el bit automáticamente */
    }
    *FIFO_DATA_REG = word;
}

/* ==========================================================================
 * API pública
 * ========================================================================== */

void audio_bridge_init(const WavHeader* header) {
    /* --- 1. Calcular cuántos bloques de 512 B necesitará el stream --- */
    uint32_t total_blocks = header->subchunk2_size / 512U;
    if (header->subchunk2_size % 512U != 0U) {
        total_blocks++;  /* Bloque fraccionario final */
    }

    /* --- 2. Enviar CMD_TRACK_START con total_blocks en el payload ---
     *        El NIOS interpreta esto como "nueva pista, espera metadatos". */
    fifo_write_word(CMD_BUILD(CMD_TRACK_START, total_blocks));

    /* --- 3. Construir y enviar el TrackMetadataPacket (5 palabras) --- */
    TrackMetadataPacket meta;
    meta.magic             = TRACK_META_MAGIC;
    meta.sample_rate       = header->sample_rate;
    meta.channels_and_bits = PACK_CHAN_BITS(header->num_channels,
                                            header->bits_per_sample);
    meta.total_audio_bytes = header->subchunk2_size;
    meta.total_blocks      = total_blocks;

    /* Serializar campo a campo — evita el cast packed* → uint32_t* que
     * genera UB de alineación en ARM (Cortex-A9 no acepta accesos desalineados
     * a uint32_t en todos los contextos).                                   */
    fifo_write_word(meta.magic);
    fifo_write_word(meta.sample_rate);
    fifo_write_word(meta.channels_and_bits);
    fifo_write_word(meta.total_audio_bytes);
    fifo_write_word(meta.total_blocks);

#ifdef QEMU_TEST
    printf("[Audio Bridge] CMD_TRACK_START enviado.\n");
    printf("[Audio Bridge] Metadatos: %lu Hz | %u ch | %u bps | %lu B | %lu bloques\n",
           (unsigned long)meta.sample_rate,
           header->num_channels,
           header->bits_per_sample,
           (unsigned long)meta.total_audio_bytes,
           (unsigned long)meta.total_blocks);
#endif
}

void audio_bridge_send_block(const uint32_t* buffer_512b, uint32_t block_index) {
    /* --- 1. Palabra de control: CMD_BLOCK_READY + índice del bloque ---
     *        El NIOS puede detectar bloques perdidos comparando el índice
     *        esperado contra el recibido.                                */
    fifo_write_word(CMD_BUILD(CMD_BLOCK_READY, block_index));

    /* --- 2. Datos PCM: 128 palabras de 32 bits (512 bytes) --- */
    for (int i = 0; i < 128; i++) {
        fifo_write_word(buffer_512b[i]);
    }

#ifdef QEMU_TEST
    printf("[Audio Bridge] Bloque %lu enviado (512 bytes, 128 words).\n",
           (unsigned long)block_index);
#endif
}

void audio_bridge_track_end(void) {
    fifo_write_word(CMD_BUILD(CMD_TRACK_END, 0U));

#ifdef QEMU_TEST
    printf("[Audio Bridge] CMD_TRACK_END enviado. Stream finalizado.\n");
#endif
}

void audio_bridge_play(void) {
    fifo_write_word(CMD_BUILD(CMD_PLAY, 0U));

#ifdef QEMU_TEST
    printf("[Audio Bridge] CMD_PLAY enviado.\n");
#endif
}

void audio_bridge_pause(void) {
    fifo_write_word(CMD_BUILD(CMD_PAUSE, 0U));

#ifdef QEMU_TEST
    printf("[Audio Bridge] CMD_PAUSE enviado.\n");
#endif
}

/* Envia un campo de texto como TRACK_TEXT_FIELD_WORDS palabras little-endian,
 * rellenado con ceros hasta TRACK_TEXT_FIELD_BYTES. */
static void send_text_field(const char* s) {
    char field[TRACK_TEXT_FIELD_BYTES];
    int i = 0;
    while (i < TRACK_TEXT_FIELD_BYTES && s[i] != '\0') { field[i] = s[i]; i++; }
    while (i < TRACK_TEXT_FIELD_BYTES) { field[i] = 0; i++; }

    for (int w = 0; w < TRACK_TEXT_FIELD_WORDS; w++) {
        uint32_t word =  (uint32_t)(uint8_t)field[w * 4 + 0]
                      | ((uint32_t)(uint8_t)field[w * 4 + 1] << 8)
                      | ((uint32_t)(uint8_t)field[w * 4 + 2] << 16)
                      | ((uint32_t)(uint8_t)field[w * 4 + 3] << 24);
        fifo_write_word(word);
    }
}

void audio_bridge_send_text(const char* title, const char* artist) {
    fifo_write_word(CMD_BUILD(CMD_TRACK_TEXT, TRACK_TEXT_WORDS));
    send_text_field(title);
    send_text_field(artist);

#ifdef QEMU_TEST
    printf("[Audio Bridge] CMD_TRACK_TEXT: \"%s\" / \"%s\"\n", title, artist);
#endif
}