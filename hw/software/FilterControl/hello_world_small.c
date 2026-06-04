#include "system.h"
#include "sys/alt_stdio.h"

#define REG32(addr) (*(volatile unsigned int *)(addr))

#define SWITCHES_REG    REG32(SWITCHES_PIO_BASE + 0x00)
#define FILTER_CTRL_REG REG32(AUDIO_FILTER_CONTROL_0_BASE + 0x00)

static void print_filter(unsigned int filter_sel)
{
    alt_putstr("Filtro seleccionado: ");

    if (filter_sel == 0)
        alt_putstr("00 bypass\n");
    else if (filter_sel == 1)
        alt_putstr("01 lowpass\n");
    else if (filter_sel == 2)
        alt_putstr("10 highpass\n");
    else if (filter_sel == 3)
        alt_putstr("11 bass boost\n");
}

int main(void)
{
    unsigned int sw_value;
    unsigned int filter_sel;
    unsigned int last_filter = 0xFFFFFFFF;

    alt_putstr("Inicio prueba filtros con Nios\n");

    FILTER_CTRL_REG = 0x0;
    alt_putstr("Registro inicializado en bypass\n");

    while (1)
    {
        sw_value = SWITCHES_REG;
        filter_sel = sw_value & 0x3;

        if (filter_sel != last_filter)
        {
            FILTER_CTRL_REG = filter_sel;
            last_filter = filter_sel;
            print_filter(filter_sel);
        }
    }

    return 0;
}
