#include "system.h"
#include "sys/alt_stdio.h"
#include "altera_up_avalon_audio_and_video_config.h"

int main(void)
{
    alt_putstr("Inicio del programa\n");
    alt_putstr("Configurando WM8731...\n");

    alt_up_av_config_dev *audio_config;

    audio_config = alt_up_av_config_open_dev(AUDIO_CONFIG_EXTERNAL_NAME);

    if (audio_config == NULL) {
        alt_putstr("ERROR: No se pudo abrir audio_config_external\n");
        while (1);
    }

    alt_putstr("Dispositivo audio_config_external encontrado\n");

    alt_up_av_config_reset(audio_config);

    alt_putstr("Esperando configuracion del codec...\n");

    while (!alt_up_av_config_read_ready(audio_config)) {
        // Espera a que termine la configuracion por I2C
    }

    alt_putstr("WM8731 configurado correctamente\n");

    while (1) {
        // El Nios queda corriendo.
        // El tono lo sigue generando tu ToneGenerator en hardware.
    }

    return 0;
}
