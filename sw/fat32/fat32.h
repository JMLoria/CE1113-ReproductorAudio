#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>

/* =============================================================================
 * Lector FAT32 de SOLO LECTURA para bare-metal.
 * Se apoya en sd_read_block() (bloques crudos de 512 B). Permite montar la
 * tarjeta, listar el directorio raíz, abrir archivos por nombre 8.3 y leerlos
 * de forma secuencial (ideal para hacer streaming de WAV al FIFO).
 *
 * Limitaciones intencionales (suficientes para el reproductor):
 *   - Solo lectura, sector de 512 B.
 *   - Nombres cortos 8.3 (sin Long File Names).
 *   - Archivos en el directorio raíz (sin recorrer subdirectorios).
 * ========================================================================== */

typedef struct {
    uint32_t first_cluster;   /* primer cluster del archivo                    */
    uint32_t size;            /* tamaño total en bytes                         */
    uint32_t bytes_left;      /* bytes que faltan por leer                     */
    uint32_t cur_cluster;     /* cluster que se está leyendo                   */
    uint32_t sector_in_clus;  /* índice de sector dentro del cluster (0..spc-1)*/
    uint32_t byte_in_sector;  /* offset dentro del sector actual (0..511)      */
    uint32_t buf[128];        /* sector actual (alineado a 32 bits)            */
    int      buf_valid;       /* 1 si buf contiene el sector actual            */
} Fat32File;

/**
 * @brief Monta la tarjeta: detecta MBR o "superfloppy" y parsea el BPB FAT32.
 * @return 0 en éxito; negativo si no hay FAT32 válido.
 */
int fat32_mount(void);

/**
 * @brief Abre un archivo del directorio raíz por nombre (acepta "SONG1.WAV").
 * @param name  Nombre en formato corto; se convierte a 8.3 internamente.
 * @param f     Estructura de archivo a inicializar.
 * @return 0 en éxito; negativo si no se encontró o no está montado.
 */
int fat32_open(const char* name, Fat32File* f);

/**
 * @brief Lee hasta 'len' bytes secuenciales del archivo.
 * @return número de bytes leídos (puede ser < len al llegar a EOF), o negativo.
 */
int fat32_read(Fat32File* f, void* dst, uint32_t len);

/**
 * @brief Imprime las entradas del directorio raíz (depuración).
 */
void fat32_list_root(void);

#endif /* FAT32_H */
