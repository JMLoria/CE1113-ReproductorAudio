#include "sd_driver.h"
#include <stdio.h> // Necesario para los printf en emulación

// Estructura de registros del Cyclone V
typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t PWREN;
    volatile uint32_t CLKDIV;
    volatile uint32_t CMDARG;
    volatile uint32_t CMD;
    volatile uint32_t RESP0;
    volatile uint32_t RINTSTS;
    volatile uint32_t BLKSIZ;
    volatile uint32_t BYTCNT;
    volatile uint32_t RESERVED[113]; // Padding hasta 0x200
    volatile uint32_t DATA;
} HPS_SDMMC_Regs;


#ifdef QEMU_TEST
    // Memoria RAM local para evitar Segmentation Faults
    static HPS_SDMMC_Regs mock_sdmmc_regs = {0};
    #define SDMMC (&mock_sdmmc_regs)
#else
    // Hardware físico de la placa
    #define SDMMC_BASE_ADDR 0xFF704000
    #define SDMMC ((HPS_SDMMC_Regs*) SDMMC_BASE_ADDR)
#endif

// Banderas del CMD
#define CMD_START        (1 << 31)
#define CMD_RESP_EXPECT  (1 << 6)

static void sd_send_cmd(uint32_t cmd_index, uint32_t arg, uint32_t flags) {
    #ifdef QEMU_TEST
        printf("[Mock Hardware] Enviando CMD%d | Argumento: 0x%08X\n", cmd_index, arg);
        // Simulamos que el hardware completó el comando (Bit 2 = Command Done)
        SDMMC->RINTSTS |= (1 << 2); 
    #else
        // Lógica física real
        while (SDMMC->CMD & CMD_START);
        SDMMC->RINTSTS = 0xFFFFFFFF;
        SDMMC->CMDARG = arg;
        SDMMC->CMD = CMD_START | flags | cmd_index;
        while (!(SDMMC->RINTSTS & (1 << 2)));
    #endif
}

void sd_init(void) {
    #ifdef QEMU_TEST
        printf("\n=== INICIANDO SECUENCIA SD ===\n");
    #endif

    // Reset
    SDMMC->CTRL = 1;
    SDMMC->PWREN = 1;

    // Máquina de estados
    sd_send_cmd(0, 0x00000000, 0); // GO_IDLE
    sd_send_cmd(8, 0x000001AA, CMD_RESP_EXPECT); // SEND_IF_COND
    
    // Simular el ciclo de ACMD41
    sd_send_cmd(55, 0x00000000, CMD_RESP_EXPECT); 
    sd_send_cmd(41, (1 << 30), CMD_RESP_EXPECT);
    
    #ifdef QEMU_TEST
        // Forzamos el bit de "Ready" para salir del ciclo en emulación
        SDMMC->RESP0 = (1 << 31); 
    #endif

    sd_send_cmd(2, 0x00000000, CMD_RESP_EXPECT); // ALL_SEND_CID
    sd_send_cmd(3, 0x00000000, CMD_RESP_EXPECT); // SEND_RELATIVE_ADDR
    
    #ifdef QEMU_TEST
        SDMMC->RESP0 = 0xABCD0000; // Simulamos una dirección RCA asignada
    #endif

    uint32_t rca = (SDMMC->RESP0 & 0xFFFF0000); 
    sd_send_cmd(7, rca, CMD_RESP_EXPECT); // SELECT_CARD
    
    SDMMC->BLKSIZ = 512;
}

void sd_read_block(uint32_t block_number, uint32_t* buffer) {
    SDMMC->BYTCNT = 512;
    sd_send_cmd(17, block_number, CMD_RESP_EXPECT);

    #ifdef QEMU_TEST
        printf("\n[Mock Hardware] Transfiriendo sector %d al buffer...\n", block_number);
        // Simulamos la llegada de 512 bytes (128 words)
        for(int i = 0; i < 128; i++) {
            buffer[i] = 0xFAFAFAFA; // Dato crudo de prueba
        }
    #else
        // Lectura real por polling del FIFO
        int words_read = 0;
        while (!(SDMMC->RINTSTS & (1 << 3))) { // Wait for Data Transfer Over
            if (SDMMC->RINTSTS & (1 << 5)) {   // RX Data Ready
                buffer[words_read++] = SDMMC->DATA;
                SDMMC->RINTSTS = (1 << 5); 
            }
        }
        while (words_read < 128) {
            buffer[words_read++] = SDMMC->DATA;
        }
    #endif
}