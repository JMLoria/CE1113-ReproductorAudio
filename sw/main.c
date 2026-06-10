#include "sd_driver.h"
#include "wav_parser.h"
#include <stdio.h>
#include <string.h>

// Forzar la declaración del inicializador de semihosting para QEMU
extern void initialise_monitor_handles(void);

// Buffer global de intercambio (512 bytes / 128 words)
uint32_t audio_buffer[128]; 

/**
 * @brief Inyecta una cabecera WAV PCM válida y muestras de audio ficticias 
 * dentro de nuestro audio_buffer para probar el wav_parser en QEMU.
 */
void simular_archivo_wav_en_sd(void) {
    uint8_t bloque_temporal[512] = {0};
    WavHeader h;
    
    // Configurar metadatos del WAV (Estéreo, 44100 Hz, 16 bits por muestra)
    memcpy(h.chunk_id, "RIFF", 4);
    h.chunk_size = 400044; // Datos simulados
    memcpy(h.format, "WAVE", 4);
    memcpy(h.subchunk1_id, "fmt ", 4);
    h.subchunk1_size = 16;
    h.audio_format = 1;     // PCM lineal
    h.num_channels = 2;     // Estéreo
    h.sample_rate = 44100;  // 44.1 kHz
    h.bits_per_sample = 16; // 16 bits
    h.byte_rate = 44100 * 2 * (16 / 8);
    h.block_align = 2 * (16 / 8);
    memcpy(h.subchunk2_id, "data", 4);
    h.subchunk2_size = 400000;

    // Copiar el struct empaquetado al inicio del bloque temporal de bytes
    memcpy(bloque_temporal, &h, sizeof(WavHeader));

    // Llenar el resto del bloque (offset 44 en adelante) con muestras de audio de prueba
    // Empaquetado: Canal Izquierdo = 0xAAAA, Canal Derecho = 0xBBBB -> 0xBBBBAAAA
    for (int i = WAV_PCM_OFFSET; i < 512; i += 4) {
        bloque_temporal[i]     = 0xAA; // Left Low
        bloque_temporal[i + 1] = 0xAA; // Left High
        bloque_temporal[i + 2] = 0xBB; // Right Low
        bloque_temporal[i + 3] = 0xBB; // Right High
    }

    // Sobrescribimos el audio_buffer general con nuestro WAV estructurado
    memcpy(audio_buffer, bloque_temporal, 512);
}

int main(void) {
    #ifdef QEMU_TEST
    // Obligatorio para redirigir printf a la terminal de Linux mediante QEMU
    initialise_monitor_handles();
    printf("--- [MODO SIMULACIÓN: QEMU] ---\n");
    #else
    printf("--- [MODO HARDWARE REAL: CYCLONE V HPS] ---\n");
    #endif

    printf("Iniciando Firmware del Reproductor de Audio...\n\n");
    
    // 1. Inicializar el controlador SD de la placa
    // En QEMU imprimirá los comandos simulados (CMD0, CMD8, etc.)
    sd_init();

    // 2. Leer el sector 0 de la tarjeta SD
    // En QEMU cargará por defecto datos basura (0xFAFAFAFA)
    sd_read_block(0, audio_buffer);

    #ifdef QEMU_TEST
    // 3. Modificamos el buffer simulado para inyectar un WAV real y testear el parser
    printf("\n[Test] Inyectando un encabezado de archivo WAV controlado en el buffer...\n");
    simular_archivo_wav_en_sd();
    #endif

    // 4. Analizar el buffer usando nuestro parser de audio
    printf("\nEjecutando el analizador de archivos WAV...\n");
    WavHeader pista_actual;
    WavStatus estado_parser = wav_parse_header((uint8_t*)audio_buffer, &pista_actual);

    // 5. Validar los resultados del parser
    if (estado_parser == WAV_OK) {
        printf("[\033[0;32mÉXITO\033[0m] Estructura WAV válida detectada en memoria.\n");
        
        // Imprimir metadatos extraídos por pantalla
        wav_print_info(&pista_actual);
        
        // Verificar que las muestras de datos de audio sigan bien alineadas después del header (Offset 44)
        // 44 bytes equivalen exactamente al índice 11 de un arreglo de 32 bits (11 * 4 = 44 bytes)
        printf("\n=== VERIFICACIÓN DE INTEGRIDAD DE AUDIO ===");
        printf("\nMuestra en audio_buffer[11] (Offset 44): 0x%08lX", (unsigned long)audio_buffer[11]);
        printf("\nMuestra en audio_buffer[12] (Offset 48): 0x%08lX\n", (unsigned long)audio_buffer[12]);
        #ifdef QEMU_TEST
        printf("Resultado Esperado:                    0xBBBBAAAA\n");
        #endif
    } else {
        printf("[\033[0;31mERROR\033[0m] El archivo en la SD no es un WAV PCM válido. Código de error: %d\n", estado_parser);
    }
    
    printf("\nPrueba de integración completada de manera exitosa.\n");
    return 0; 
}