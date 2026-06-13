#include "sd_driver.h"
#include "wav_parser.h"
#include <stdio.h>

int main(void) {
    uint32_t buffer[128];

    // 1. Inicializar SD
    sd_init();

    // 2. Leer sector 0 (header WAV)
    sd_read_block(0, buffer);

    // 3. Parsear y mostrar metadatos — sin tocar la FPGA
    WavHeader header;
    WavStatus status = wav_parse_header((uint8_t*)buffer, &header);

    if (status == WAV_OK) {
        wav_print_info(&header);
    } else {
        printf("ERROR: WAV inválido, código %d\n", status);
    }

    return 0;
}