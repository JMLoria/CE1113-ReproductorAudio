#ifndef SD_DRIVER_H
#define SD_DRIVER_H

#include <stdint.h>

/**
 * @brief Inicializa el controlador SD/MMC del HPS y la tarjeta SD física.
 * Ejecuta la máquina de estados de identificación (CMD0, CMD8, ACMD41, etc.)
 * y configura el hardware para transferencias de bloques de 512 bytes.
 */
void sd_init(void);

/**
 * @brief Lee un bloque crudo de 512 bytes desde la tarjeta SD.
 * @param block_number El sector lógico a leer (LBA).
 * @param buffer Puntero a un arreglo de al menos 128 elementos (128 x 32 bits = 512 bytes).
 */
void sd_read_block(uint32_t block_number, uint32_t* buffer);

/**
 * @brief Alternativa a sd_init() cuando U-Boot YA inicializo el controlador
 * SD/MMC (cargo la app desde la tarjeta). Evita re-ejecutar la secuencia
 * CMD0..CMD7 (que en hardware real cuelga porque CMD0 resetea la tarjeta).
 * Solo asume la tarjeta lista en modo transfer y de tipo SDHC/SDXC.
 */
void sd_use_preinit(void);

#endif // SD_DRIVER_H