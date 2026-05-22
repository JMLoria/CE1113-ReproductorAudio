#include "sd_driver.h"
#include <stdio.h>

uint32_t audio_buffer[128]; 

int main(void) {
    printf("--- Firmware SoC Audio Reproductor Iniciado ---\n");
    
    // 1. Inicializar hardware
    sd_init();

    // 2. Leer un bloque crudo
    sd_read_block(0, audio_buffer);

    // 3. Verificar que los datos llegaron a la memoria
    printf("\n=== RESULTADO DE LA LECTURA ===\n");
    printf("Palabra 0 del buffer: 0x%08X\n", audio_buffer[0]);
    printf("Palabra 1 del buffer: 0x%08X\n", audio_buffer[1]);
    
    printf("\nPrueba de Bare Metal completada exitosamente.\n");
    
    return 0; 
}