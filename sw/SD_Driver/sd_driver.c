#include "sd_driver.h"
#include <stdbool.h>

// Definición del Mapa de Memoria (Privado para el driver)
#define SDMMC_BASE_ADDR 0xFF704000

typedef struct {
    volatile uint32_t CTRL;       // 0x000
    volatile uint32_t PWREN;      // 0x004
    volatile uint32_t CLKDIV;     // 0x008
    volatile uint32_t CLKSRC;     // 0x00C
    volatile uint32_t CLKENA;     // 0x010
    volatile uint32_t TMOUT;      // 0x014
    volatile uint32_t CTYPE;      // 0x018
    volatile uint32_t BLKSIZ;     // 0x01C
    volatile uint32_t BYTCNT;     // 0x020
    volatile uint32_t INTMASK;    // 0x024
    volatile uint32_t CMDARG;     // 0x028
    volatile uint32_t CMD;        // 0x02C
    volatile uint32_t RESP0;      // 0x030
    volatile uint32_t RESP1;      // 0x034
    volatile uint32_t RESP2;      // 0x038
    volatile uint32_t RESP3;      // 0x03C
    volatile uint32_t MINTSTS;    // 0x040
    volatile uint32_t RINTSTS;    // 0x044
    volatile uint32_t STATUS;     // 0x048
    volatile uint32_t FIFOTH;     // 0x04C
    volatile uint32_t RESERVED[108]; 
    volatile uint32_t DATA;       // 0x200
} HPS_SDMMC_Regs;

#define SDMMC ((HPS_SDMMC_Regs*) SDMMC_BASE_ADDR)

// Banderas internas del CMD
#define CMD_START        (1 << 31)
#define CMD_USE_HOLD_REG (1 << 29) 
#define CMD_UPDATE_CLK   (1 << 21)
#define CMD_SEND_INIT    (1 << 15)
#define CMD_WAIT_PRVDATA (1 << 13)
#define CMD_DATA_EXPECT  (1 << 9)
#define CMD_CHECK_CRC    (1 << 8)
#define CMD_RESP_LONG    (1 << 7)
#define CMD_RESP_EXPECT  (1 << 6)


// Funciones Privadas (Helper)
static void sd_send_cmd(uint32_t cmd_index, uint32_t arg, uint32_t flags) {
    while (SDMMC->CMD & CMD_START);

    SDMMC->RINTSTS = 0xFFFFFFFF; // Clear interrupts
    SDMMC->CMDARG = arg;
    SDMMC->CMD = CMD_START | CMD_USE_HOLD_REG | flags | cmd_index;

    // Esperar a que Command Done (Bit 2) se active
    while (!(SDMMC->RINTSTS & (1 << 2)));
}


// Implementación de la API Pública
void sd_init(void) {
    SDMMC->CTRL = 1;
    while (SDMMC->CTRL & 1); 

    SDMMC->PWREN = 1; 
    
    // CMD0
    sd_send_cmd(0, 0x00000000, CMD_SEND_INIT);

    // CMD8
    sd_send_cmd(8, 0x000001AA, CMD_RESP_EXPECT | CMD_CHECK_CRC);

    // ACMD41
    uint32_t resp = 0;
    do {
        sd_send_cmd(55, 0x00000000, CMD_RESP_EXPECT | CMD_CHECK_CRC);
        sd_send_cmd(41, (1 << 30), CMD_RESP_EXPECT);
        resp = SDMMC->RESP0;
    } while ((resp & (1 << 31)) == 0); 

    // CMD2
    sd_send_cmd(2, 0x00000000, CMD_RESP_EXPECT | CMD_RESP_LONG | CMD_CHECK_CRC);

    // CMD3
    sd_send_cmd(3, 0x00000000, CMD_RESP_EXPECT | CMD_CHECK_CRC);
    uint32_t rca = (SDMMC->RESP0 & 0xFFFF0000); 

    // CMD7
    sd_send_cmd(7, rca, CMD_RESP_EXPECT | CMD_CHECK_CRC);

    SDMMC->BLKSIZ = 512;
}

void sd_read_block(uint32_t block_number, uint32_t* buffer) {
    SDMMC->BYTCNT = 512;
    sd_send_cmd(17, block_number, CMD_RESP_EXPECT | CMD_CHECK_CRC | CMD_DATA_EXPECT | CMD_WAIT_PRVDATA);

    int words_read = 0;

    // Polling hasta Data Transfer Over (Bit 3)
    while (!(SDMMC->RINTSTS & (1 << 3))) {
        // RX Data Ready (Bit 5)
        if (SDMMC->RINTSTS & (1 << 5)) {
            buffer[words_read++] = SDMMC->DATA;
            SDMMC->RINTSTS = (1 << 5); 
        }
    }
    
    // Vaciar el resto del FIFO
    while (words_read < 128) {
        buffer[words_read++] = SDMMC->DATA;
    }
}