/*
 * ipc_protocol.h
 *
 * Constantes del protocolo IPC sobre el FIFO HPS -> NIOS.
 * DEBE coincidir byte a byte con sw/audio_bridge/audio_bridge.h (lado HPS).
 *
 * Formato de la palabra de control (32 bits):
 *   [31:24] opcode  |  [23:0] payload
 *
 *   CMD_TRACK_START : nueva pista. Le siguen TRACK_META_WORDS palabras
 *                     (TrackMetadataPacket: magic, sample_rate,
 *                      channels|bits, total_bytes, total_blocks).
 *   CMD_TRACK_TEXT  : metadatos de texto. Le siguen TRACK_TEXT_WORDS palabras
 *                     (titulo: 12 palabras, artista: 12 palabras, LE).
 *   CMD_BLOCK_READY : bloque PCM. Le siguen 128 palabras (512 B).
 *   CMD_TRACK_END   : fin de pista.
 *   CMD_PLAY/PAUSE  : ordenes de reproduccion (aqui se ignoran: el play/pausa
 *                     lo controlan los botones fisicos del NIOS).
 */
#ifndef IPC_PROTOCOL_H
#define IPC_PROTOCOL_H

#define CMD_OPCODE_MASK     0xFF000000U
#define CMD_TRACK_START     0xA0000000U
#define CMD_BLOCK_READY     0xB0000000U
#define CMD_TRACK_END       0xC0000000U
#define CMD_PLAY            0xD0000000U
#define CMD_PAUSE           0xE0000000U
#define CMD_TRACK_TEXT      0xF0000000U
#define CMD_PAYLOAD(word)   ((word) & 0x00FFFFFFU)

/* Paquete de metadatos numericos (tras CMD_TRACK_START) */
#define TRACK_META_MAGIC    0xAD100001U
#define TRACK_META_WORDS    5

/* Paquete de texto (tras CMD_TRACK_TEXT): titulo + artista, longitud fija */
#define TRACK_TEXT_FIELD_BYTES  48
#define TRACK_TEXT_FIELD_WORDS  12               /* 48 / 4 */
#define TRACK_TEXT_WORDS        (2 * TRACK_TEXT_FIELD_WORDS)

/* Tamano de un bloque PCM en palabras (512 B / 4) */
#define PCM_BLOCK_WORDS     128

#endif /* IPC_PROTOCOL_H */
