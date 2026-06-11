#include "sd_driver.h"
#include "wav_parser.h"
#include "audio_bridge.h"
#include <stdio.h>
#include <string.h>

extern void initialise_monitor_handles(void);

uint32_t audio_buffer[128]; 

void simular_archivo_wav_en_sd(void) {
    uint8_t bloque_temporal[512] = {0};
    WavHeader h;
    
    memcpy(h.chunk_id, "RIFF", 4);
    h.chunk_size = 400044; 
    memcpy(h.format, "WAVE", 4);
    memcpy(h.subchunk1_id, "fmt ", 4);
    h.subchunk1_size = 16;
    h.audio_format = 1;     
    h.num_channels = 2;     
    h.sample_rate = 44100;  
    h.bits_per_sample = 16; 
    h.byte_rate = 44100 * 2 * (16 / 8);
    h.block_align = 2 * (16 / 8);
    memcpy(h.subchunk2_id, "data", 4);
    // Para no hacer la simulación infinita en QEMU, simulamos que el audio mide 4096 bytes (8 bloques de 512 bytes)
    h.subchunk2_size = 4096; 

    memcpy(bloque_temporal, &h, sizeof(WavHeader));

    for (int i = WAV_PCM_OFFSET; i < 512; i += 4) {
        bloque_temporal[i]     = 0xAA; 
        bloque_temporal[i + 1] = 0xAA; 
        bloque_temporal[i + 2] = 0xBB; 
        bloque_temporal[i + 3] = 0xBB; 
    }

    memcpy(audio_buffer, bloque_temporal, 512);
}

int main(void) {
    #ifdef QEMU_TEST
    initialise_monitor_handles();
    printf("--- [MODO SIMULACIÓN: QEMU] ---\n");
    #else
    printf("--- [MODO HARDWARE REAL: CYCLONE V HPS] ---\n");
    #endif

    printf("Iniciando Firmware del Reproductor de Audio...\n\n");
    
    // 1. Inicializar el controlador SD
    sd_init();

    // 2. Leer el sector 0 (Donde reside el encabezado de la pista)
    sd_read_block(0, audio_buffer);

    #ifdef QEMU_TEST
    simular_archivo_wav_en_sd();
    #endif

    // 3. Analizar el encabezado WAV
    printf("\nEjecutando el analizador de archivos WAV...\n");
    WavHeader pista_actual;
    WavStatus estado_parser = wav_parse_header((uint8_t*)audio_buffer, &pista_actual);

    if (estado_parser == WAV_OK) {
        printf("[\033[0;32mÉXITO\033[0m] Estructura WAV válida detectada.\n");
        wav_print_info(&pista_actual);
        
        // 4. Inicializar puente de audio con la FPGA.
        // audio_bridge_init() transmite CMD_TRACK_START + metadatos al NIOS.
        // audio_bridge_play() envía CMD_PLAY para arrancar la reproducción.
        printf("\n=== INICIANDO TRANSFERENCIA A LA FPGA ===\n");
        audio_bridge_init(&pista_actual);
        audio_bridge_play();

        // 5. Calcular cuántos bloques de datos de audio debemos procesar
        // Cada bloque de la SD tiene 512 bytes.
        uint32_t bytes_de_audio = pista_actual.subchunk2_size;
        uint32_t bloques_totales = bytes_de_audio / 512;
        if (bytes_de_audio % 512 != 0) bloques_totales++; // Bloque fraccionario restante

        printf("[Reproductor] Detectados %ld bloques de audio para reproducir.\n", (unsigned long)bloques_totales);

        // Enviamos el primer bloque (Sector 0) que ya contiene el residuo inicial de datos de audio
        printf("\n[Reproductor] Enviando Bloque Inicial (Sector 0)...\n");
        audio_bridge_send_block(audio_buffer, 0);

        // Bucle de reproducción: Lee secuencialmente de la SD y envía a la FPGA
        // Empezamos en el sector 1 ya que el 0 ya fue transmitido
        for (uint32_t bloque_actual = 1; bloque_actual <= bloques_totales; bloque_actual++) {
            printf("[Reproductor] Procesando Bloque %ld/%ld (SD LBA: %ld)...\n", 
                   (unsigned long)bloque_actual, (unsigned long)bloques_totales, (unsigned long)bloque_actual);
            
            // Leer siguiente bloque físico de la SD
            sd_read_block(bloque_actual, audio_buffer);
            
            // Enviar datos al Nios a través de nuestro puente unificado
            audio_bridge_send_block(audio_buffer, bloque_actual);
        }

        // Señalizar al NIOS que no vienen más bloques
        audio_bridge_track_end();

        printf("\n[\033[0;32mFIN\033[0m] Se han enviado todos los bloques. Canción finalizada con éxito.\n");

    } else {
        printf("[\033[0;31mERROR\033[0m] Archivo WAV inválido. Código: %d\n", estado_parser);
    }
    
    printf("\nPrueba de ejecución en bucle completada.\n");
    return 0; 
}