#include "sd_driver.h"
#include <stdio.h> // Necesario para los printf en emulación

/* ==========================================================================
 * Driver SD/MMC para el HPS del Cyclone V (Altera/Intel DE1-SoC)
 *
 * El controlador embebido en el HPS es un Synopsys DesignWare Mobile Storage
 * Host Controller (dw_mmc). El mapa de registros y la secuencia de comandos
 * siguen el "Cyclone V Hard Processor System Technical Reference Manual"
 * (capítulo SD/MMC Controller) y el driver de referencia de Linux (dw_mmc).
 *
 * Modo de operación: lectura por polling del FIFO interno (SIN DMA), bus de
 * 1 bit. Es la configuración más simple y robusta para el bring-up; una vez
 * verificada la lectura de bloques se puede escalar a 4 bits + DMA.
 * ========================================================================== */

/* -------------------------------------------------------------------------
 * Mapa de registros (offsets reales desde la base 0xFF704000)
 * ------------------------------------------------------------------------- */
typedef struct {
    volatile uint32_t CTRL;        /* 0x000 Control                          */
    volatile uint32_t PWREN;       /* 0x004 Power Enable                      */
    volatile uint32_t CLKDIV;      /* 0x008 Clock Divider                     */
    volatile uint32_t CLKSRC;      /* 0x00C Clock Source                      */
    volatile uint32_t CLKENA;      /* 0x010 Clock Enable                      */
    volatile uint32_t TMOUT;       /* 0x014 Timeout                           */
    volatile uint32_t CTYPE;       /* 0x018 Card Type (ancho de bus)          */
    volatile uint32_t BLKSIZ;      /* 0x01C Block Size                        */
    volatile uint32_t BYTCNT;      /* 0x020 Byte Count                        */
    volatile uint32_t INTMASK;     /* 0x024 Interrupt Mask                    */
    volatile uint32_t CMDARG;      /* 0x028 Command Argument                  */
    volatile uint32_t CMD;         /* 0x02C Command                           */
    volatile uint32_t RESP0;       /* 0x030 Response 0                        */
    volatile uint32_t RESP1;       /* 0x034 Response 1                        */
    volatile uint32_t RESP2;       /* 0x038 Response 2                        */
    volatile uint32_t RESP3;       /* 0x03C Response 3                        */
    volatile uint32_t MINTSTS;     /* 0x040 Masked Interrupt Status           */
    volatile uint32_t RINTSTS;     /* 0x044 Raw Interrupt Status              */
    volatile uint32_t STATUS;      /* 0x048 Status (FIFO count, busy, etc.)   */
    volatile uint32_t FIFOTH;      /* 0x04C FIFO Threshold                    */
    volatile uint32_t CDETECT;     /* 0x050 Card Detect                       */
    volatile uint32_t WRTPRT;      /* 0x054 Write Protect                     */
    volatile uint32_t GPIO;        /* 0x058 GPIO                              */
    volatile uint32_t TCBCNT;      /* 0x05C Transferred CIU Byte Count        */
    volatile uint32_t TBBCNT;      /* 0x060 Transferred Host/Bus Byte Count   */
    volatile uint32_t DEBNCE;      /* 0x064 Debounce Count                    */
    volatile uint32_t USRID;       /* 0x068 User ID                           */
    volatile uint32_t VERID;       /* 0x06C Version ID                        */
    volatile uint32_t HCON;        /* 0x070 Hardware Configuration            */
    volatile uint32_t UHS_REG;     /* 0x074 UHS-1                             */
    volatile uint32_t RST_N;       /* 0x078 Hardware Reset                    */
    volatile uint32_t RESERVED[97];/* 0x07C .. 0x1FC                          */
    volatile uint32_t DATA;        /* 0x200 Data FIFO (puerto de lectura)     */
} HPS_SDMMC_Regs;


#ifdef QEMU_TEST
    // Memoria RAM local para evitar Segmentation Faults en simulación
    static HPS_SDMMC_Regs mock_sdmmc_regs = {0};
    #define SDMMC (&mock_sdmmc_regs)
#else
    // Hardware físico de la placa
    #define SDMMC_BASE_ADDR 0xFF704000
    #define SDMMC ((HPS_SDMMC_Regs*) SDMMC_BASE_ADDR)
#endif

/* -------------------------------------------------------------------------
 * Bits del registro CTRL
 * ------------------------------------------------------------------------- */
#define CTRL_CONTROLLER_RESET   (1u << 0)
#define CTRL_FIFO_RESET         (1u << 1)
#define CTRL_DMA_RESET          (1u << 2)
#define CTRL_INT_ENABLE         (1u << 4)

/* -------------------------------------------------------------------------
 * Bits del registro CLKENA
 * ------------------------------------------------------------------------- */
#define CLKENA_CCLK_ENABLE      (1u << 0)   /* Habilita el reloj de la tarjeta 0 */

/* -------------------------------------------------------------------------
 * Bits del registro CMD
 * ------------------------------------------------------------------------- */
#define CMD_START               (1u << 31)  /* Inicia el comando                  */
#define CMD_USE_HOLD_REG        (1u << 29)  /* Usa el hold register (recomendado) */
#define CMD_UPD_CLK             (1u << 21)  /* Solo actualiza regs de reloj       */
#define CMD_INIT                (1u << 15)  /* Envía secuencia de 80 relojes init */
#define CMD_PRV_DAT_WAIT        (1u << 13)  /* Espera fin de datos previos        */
#define CMD_DAT_WR              (1u << 10)  /* 1 = escritura (0 = lectura)        */
#define CMD_DAT_EXP             (1u << 9)   /* El comando transfiere datos        */
#define CMD_RESP_CRC            (1u << 8)   /* Verifica CRC de la respuesta       */
#define CMD_RESP_LONG           (1u << 7)   /* Respuesta larga (136 bits, R2)     */
#define CMD_RESP_EXP            (1u << 6)   /* Se espera respuesta                */
#define CMD_INDEX(n)            ((n) & 0x3Fu)

/* -------------------------------------------------------------------------
 * Bits del registro RINTSTS (Raw Interrupt Status)
 * ------------------------------------------------------------------------- */
#define INT_CD                  (1u << 0)   /* Card detect                        */
#define INT_RESP_ERR            (1u << 1)   /* Error de respuesta                 */
#define INT_CMD_DONE            (1u << 2)   /* Comando completado                 */
#define INT_DTO                 (1u << 3)   /* Data Transfer Over                 */
#define INT_TXDR                (1u << 4)   /* Transmit FIFO Data Request         */
#define INT_RXDR                (1u << 5)   /* Receive FIFO Data Request          */
#define INT_RCRC                (1u << 6)   /* Response CRC error                 */
#define INT_DCRC                (1u << 7)   /* Data CRC error                     */
#define INT_RTO                 (1u << 8)   /* Response Timeout                   */
#define INT_DRTO                (1u << 9)   /* Data Read Timeout                  */
#define INT_HTO                 (1u << 10)  /* Data starvation host timeout       */
#define INT_FRUN                (1u << 11)  /* FIFO under/overrun                 */
#define INT_HLE                 (1u << 12)  /* Hardware locked write error        */
#define INT_SBE                 (1u << 13)  /* Start bit error                    */
#define INT_EBE                 (1u << 15)  /* End bit error                      */

#define INT_RESP_ERROR_MASK     (INT_RESP_ERR | INT_RCRC | INT_RTO)
#define INT_DATA_ERROR_MASK     (INT_DCRC | INT_DRTO | INT_SBE | INT_EBE | INT_FRUN)

/* -------------------------------------------------------------------------
 * Bits del registro STATUS
 * ------------------------------------------------------------------------- */
#define STATUS_FIFO_EMPTY       (1u << 2)
#define STATUS_FIFO_FULL        (1u << 3)
#define STATUS_DATA_BUSY        (1u << 9)
#define STATUS_FIFO_COUNT(x)    (((x) >> 17) & 0x1FFFu)  /* nº de palabras en FIFO */

/* -------------------------------------------------------------------------
 * Divisores de reloj. cclk_in del controlador = 200 MHz (ver soc_system.qsys
 * / hps.xml: sdmmc_clk_hz = 200000000). f_card = cclk_in / (2 * CLKDIV).
 * ------------------------------------------------------------------------- */
#define CLKDIV_INIT     250u   /* 200 MHz / (2*250) = 400 kHz  (identificación) */
#define CLKDIV_XFER     4u     /* 200 MHz / (2*4)   = 25 MHz   (default speed)  */

/* Límite de iteraciones para evitar cuelgues si la tarjeta no responde */
#define SD_POLL_TIMEOUT 5000000u

/* Patear el watchdog L4WD0 (CRR @ 0xFFD0200C) dentro de los bucles de espera.
 * Sin esto, una espera de ~1s puede acumularse con otras y disparar el reset
 * del HPS a los 5s. Solo se usa en hardware real (no en QEMU_TEST). */
#ifndef QEMU_TEST
#define SD_WDT_PET()  (*(volatile uint32_t*)0xFFD0200CU = 0x76u)
#else
#define SD_WDT_PET()  ((void)0)
#endif

/* Estado del driver detectado durante la inicialización */
static int      sd_is_sdhc = 0;   /* 1 si la tarjeta es de alta capacidad (SDHC/SDXC) */
static uint32_t sd_rca     = 0;   /* Relative Card Address asignada por la tarjeta     */

/* -------------------------------------------------------------------------
 * Envía el comando de actualización de reloj (CMD52 con bit UPD_CLK) y espera
 * a que se complete. 
 * ------------------------------------------------------------------------- */
static void sd_update_clocks(void) {
#ifndef QEMU_TEST
    uint32_t guard = SD_POLL_TIMEOUT;
    SDMMC->CMD = CMD_START | CMD_UPD_CLK | CMD_PRV_DAT_WAIT;
    while ((SDMMC->CMD & CMD_START) && --guard) { SD_WDT_PET(); }
#endif
}

/* -------------------------------------------------------------------------
 * Configura y habilita el reloj de la tarjeta a la frecuencia indicada.
 * ------------------------------------------------------------------------- */
static void sd_set_clock(uint32_t divider) {
    // 1. Deshabilitar el reloj antes de tocar el divisor
    SDMMC->CLKENA = 0;
    sd_update_clocks();

    // 2. Programar divisor y fuente
    SDMMC->CLKDIV = divider;
    SDMMC->CLKSRC = 0;
    sd_update_clocks();

    // 3. Habilitar el reloj de la tarjeta
    SDMMC->CLKENA = CLKENA_CCLK_ENABLE;
    sd_update_clocks();
}

/* -------------------------------------------------------------------------
 * Envía un comando al controlador.
 *   index : número de comando (0-63)
 *   arg   : argumento de 32 bits
 *   flags : bits adicionales del registro CMD (CMD_RESP_EXP, CMD_RESP_LONG,
 *           CMD_RESP_CRC, CMD_DAT_EXP, ...). START/USE_HOLD_REG/PRV_DAT_WAIT
 *           e INDEX se añaden automáticamente.
 * Devuelve 0 en éxito, -1 si hubo timeout o error de respuesta.
 * ------------------------------------------------------------------------- */
static int sd_send_cmd(uint32_t index, uint32_t arg, uint32_t flags) {
#ifdef QEMU_TEST
    printf("[Mock Hardware] Enviando CMD%lu | Argumento: 0x%08lX\n",
           (unsigned long)index, (unsigned long)arg);
    (void)flags;
    return 0;
#else
    uint32_t guard;

    // Limpiar estados de interrupción previos
    SDMMC->RINTSTS = 0xFFFFFFFF;

    // Cargar argumento y disparar el comando
    SDMMC->CMDARG = arg;
    SDMMC->CMD = CMD_START | CMD_USE_HOLD_REG | CMD_PRV_DAT_WAIT |
                 flags | CMD_INDEX(index);

    // Esperar a que la CIU acepte el comando (START se limpia)
    guard = SD_POLL_TIMEOUT;
    while ((SDMMC->CMD & CMD_START) && --guard) { SD_WDT_PET(); }
    if (guard == 0) return -1;

    // Si no se espera respuesta, se termina aqui 
    if (!(flags & CMD_RESP_EXP)) {
        guard = SD_POLL_TIMEOUT;
        while (!(SDMMC->RINTSTS & INT_CMD_DONE) && --guard) { SD_WDT_PET(); }
        return (guard == 0) ? -1 : 0;
    }

    // Esperar Command Done o error de respuesta
    guard = SD_POLL_TIMEOUT;
    while (!(SDMMC->RINTSTS & (INT_CMD_DONE | INT_RESP_ERROR_MASK)) && --guard) { SD_WDT_PET(); }
    if (guard == 0) return -1;

    // Timeout / CRC de respuesta = fallo (salvo que el llamante ignore CRC)
    if (SDMMC->RINTSTS & INT_RTO) return -1;
    if ((flags & CMD_RESP_CRC) && (SDMMC->RINTSTS & INT_RCRC)) return -1;

    return 0;
#endif
}

void sd_init(void) {
    int i;

#ifdef QEMU_TEST
    printf("\n=== INICIANDO SECUENCIA SD (SIMULACIÓN) ===\n");
#endif

    sd_is_sdhc = 0;
    sd_rca     = 0;

    // 1. Reset del controlador y del FIFO
    SDMMC->PWREN = 0;
    SDMMC->CTRL  = CTRL_CONTROLLER_RESET | CTRL_FIFO_RESET | CTRL_DMA_RESET;
#ifndef QEMU_TEST
    {
        uint32_t guard = SD_POLL_TIMEOUT;
        while ((SDMMC->CTRL & (CTRL_CONTROLLER_RESET | CTRL_FIFO_RESET |
                               CTRL_DMA_RESET)) && --guard) { }
    }
#endif

    // 2. Energizar la tarjeta y desenmascarar (modo polling: sin IRQs a la CPU)
    SDMMC->PWREN   = 1;
    SDMMC->INTMASK = 0;
    SDMMC->RINTSTS = 0xFFFFFFFF;
    SDMMC->TMOUT   = 0xFFFFFFFF;   // timeouts máximos
    SDMMC->CTYPE   = 0;            // bus de 1 bit para identificación
    SDMMC->BLKSIZ  = 512;
    SDMMC->FIFOTH  = (0x2u << 28) | (0x7Fu << 16) | 0x80u; // MSize/RX/TX razonables

    // 3. Reloj lento (~400 kHz) para la fase de identificación
    sd_set_clock(CLKDIV_INIT);

    // 4. CMD0: GO_IDLE_STATE (con secuencia de inicialización de 80 relojes)
    sd_send_cmd(0, 0x00000000, CMD_INIT);

    // 5. CMD8: SEND_IF_COND (0x1AA = VHS 2.7-3.6V + patrón de chequeo 0xAA)
    //    Si la tarjeta responde con el mismo patrón es SD v2.0+.
    sd_send_cmd(8, 0x000001AA, CMD_RESP_EXP | CMD_RESP_CRC);

    // 6. ACMD41: bucle de inicialización hasta que la tarjeta deje de estar busy.
    //    HCS=bit30 (soporta alta capacidad), ventana de voltaje 0xFF8000.
    {
        uint32_t ocr;
        uint32_t guard = SD_POLL_TIMEOUT;
        do {
            sd_send_cmd(55, 0x00000000, CMD_RESP_EXP | CMD_RESP_CRC); // APP_CMD
            // ACMD41 usa respuesta R3 (OCR): SIN verificación de CRC.
            sd_send_cmd(41, 0x40FF8000, CMD_RESP_EXP);
            ocr = SDMMC->RESP0;
#ifdef QEMU_TEST
            ocr = (1u << 31) | (1u << 30); // mock: tarjeta lista y SDHC
#endif
        } while (!(ocr & (1u << 31)) && --guard); // bit31 = power-up done

        sd_is_sdhc = (ocr & (1u << 30)) ? 1 : 0;  // bit30 (CCS) = 1 -> SDHC/SDXC
    }

    // 7. CMD2: ALL_SEND_CID (respuesta larga R2)
    sd_send_cmd(2, 0x00000000, CMD_RESP_EXP | CMD_RESP_LONG | CMD_RESP_CRC);

    // 8. CMD3: SEND_RELATIVE_ADDR — la tarjeta devuelve su RCA en RESP0[31:16]
    sd_send_cmd(3, 0x00000000, CMD_RESP_EXP | CMD_RESP_CRC);
#ifdef QEMU_TEST
    SDMMC->RESP0 = 0xABCD0000; // mock: RCA simulada
#endif
    sd_rca = SDMMC->RESP0 & 0xFFFF0000u;

    // 9. CMD7: SELECT_CARD (pasa la tarjeta a estado "transfer")
    sd_send_cmd(7, sd_rca, CMD_RESP_EXP | CMD_RESP_CRC);

    // 10. Subir el reloj a velocidad de transferencia y fijar tamaño de bloque
    sd_set_clock(CLKDIV_XFER);
    SDMMC->BLKSIZ = 512;

#ifdef QEMU_TEST
    printf("[Mock Hardware] Init completo. SDHC=%d, RCA=0x%08lX\n",
           sd_is_sdhc, (unsigned long)sd_rca);
#endif
    (void)i;
}

void sd_use_preinit(void) {
    /*Asumimos SDHC/SDXC (tarjetas > 2 GB usan direccion de bloque/LBA). */
    sd_is_sdhc = 1;
    sd_rca     = 0;
#ifndef QEMU_TEST
    /* U-Boot deja el controlador en modo DMA interno (los datos van al IDMAC,
     * NO al FIFO del host). sd_read_block lee del FIFO, se pasa el
     * controlador a modo FIFO/PIO y reseteamos el FIFO. */
    SDMMC->CTRL &= ~((1u << 25) | (1u << 5));   /* USE_INTERNAL_DMAC=0, DMA_ENABLE=0 */
    SDMMC->CTRL |= CTRL_FIFO_RESET;
    { uint32_t g = SD_POLL_TIMEOUT; while ((SDMMC->CTRL & CTRL_FIFO_RESET) && --g) { } }
    SDMMC->BLKSIZ = 512;
#endif
}

void sd_read_block(uint32_t block_number, uint32_t* buffer) {
    // En SDHC/SDXC el argumento de CMD17 es la dirección de BLOQUE (LBA).
    // En tarjetas de capacidad estándar es la dirección en BYTES.
    uint32_t addr = sd_is_sdhc ? block_number : (block_number * 512u);

#ifdef QEMU_TEST
    sd_send_cmd(17, addr, CMD_RESP_EXP);
    printf("\n[Mock Hardware] Transfiriendo sector %lu al buffer...\n",
           (unsigned long)block_number);
    for (int i = 0; i < 128; i++) {
        buffer[i] = 0xFAFAFAFA; // Dato crudo de prueba
    }
#else
    uint32_t words_read = 0;
    uint32_t guard = SD_POLL_TIMEOUT;

    // Se espera a que el controlador termine cualquier transferencia de datos
    // previa y patear el watchdog.
    while ((SDMMC->STATUS & STATUS_DATA_BUSY) && --guard) {
        *(volatile uint32_t*)0xFFD0200CU = 0x76u;   // watchdog L4WD0 restart
    }

    // Reset del FIFO antes de la transferencia y limpieza de flags
    guard = SD_POLL_TIMEOUT;
    SDMMC->CTRL   |= CTRL_FIFO_RESET;
    while ((SDMMC->CTRL & CTRL_FIFO_RESET) && --guard) { }
    SDMMC->RINTSTS = 0xFFFFFFFF;

    // Programar tamaños de la transferencia
    SDMMC->BLKSIZ = 512;
    SDMMC->BYTCNT = 512;

    // CMD17: READ_SINGLE_BLOCK (espera datos, lectura)
    if (sd_send_cmd(17, addr, CMD_RESP_EXP | CMD_RESP_CRC | CMD_DAT_EXP) != 0) {
        return; // error de comando; el buffer queda sin tocar
    }

    // Drenar el FIFO por polling hasta Data Transfer Over (DTO)
    guard = SD_POLL_TIMEOUT;
    while (words_read < 128 && --guard) {
        uint32_t status = SDMMC->STATUS;

        // Vaciar todas las palabras disponibles en el FIFO
        while (!(status & STATUS_FIFO_EMPTY) && words_read < 128) {
            buffer[words_read++] = SDMMC->DATA;
            status = SDMMC->STATUS;
        }

        uint32_t ints = SDMMC->RINTSTS;
        if (ints & INT_DATA_ERROR_MASK) {
            return; // error de datos (CRC/timeout/etc.)
        }
        if (ints & INT_DTO) {
            // Transferencia terminada: vaciar el residuo que quede en el FIFO
            while (!(SDMMC->STATUS & STATUS_FIFO_EMPTY) && words_read < 128) {
                buffer[words_read++] = SDMMC->DATA;
            }
            break;
        }
    }
#endif
}
