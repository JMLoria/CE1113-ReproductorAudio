#include "sd_driver.h"
#include "fat32.h"
#include "wav_parser.h"
#include "audio_bridge.h"
#include <stdio.h>
#include <string.h>

/* =============================================================================
 * Reproductor de audio — app BARE-METAL del HPS (ARM Cortex-A9).
 *
 *   SD (sd_driver) -> FAT32 -> WAV (wav_scan) -> FIFO IPC (audio_bridge) -> NIOS
 *
 * El HPS es el DUEÑO de la navegacion de pistas: lee KEY2 (siguiente) y KEY3
 * (anterior) directamente por el puente ligero HPS->FPGA y cambia la cancion
 * que streamea. Play/Pausa/Stop los maneja el NIOS (gatea el consumo del FIFO).
 * El streaming es NO bloqueante para que los botones respondan siempre.
 * ========================================================================== */

/* Botones fisicos via el puente ligero (activo-bajo: presionado = 0).
 * buttons_pio esta mapeado en 0xFF200000 + 0x50000 (ver soc_system.qsys). */
#define HPS_LW_BASE     0xFF200000U
#define BUTTONS_DATA    (*(volatile uint32_t*)(HPS_LW_BASE + 0x50000U))
#define KEY_NEXT_MASK   (1U << 2)   /* KEY2 = siguiente */
#define KEY_PREV_MASK   (1U << 3)   /* KEY3 = anterior  */

/* Watchdog L4 del HPS: el BootROM lo deja HABILITADO. Si la app no lo reinicia
 * periodicamente, el watchdog resetea el HPS (reboot loop cada pocos segundos).
 * Escribir 0x76 al Counter Restart Register (CRR) lo "patea". */
#define L4WD0_CRR   (*(volatile uint32_t*)0xFFD0200CU)
#define WDT_KICK    0x76U
static inline void wdt_pet(void) { L4WD0_CRR = WDT_KICK; }

static const char* PLAYLIST[] = {
    "SONG1.WAV", "SONG2.WAV", "SONG3.WAV", "SONG4.WAV", "SONG5.WAV",
    "SONG6.WAV", "SONG7.WAV", "SONG8.WAV", "SONG9.WAV", "SONG10.WAV",
};
#define PLAYLIST_LEN (sizeof(PLAYLIST) / sizeof(PLAYLIST[0]))

/* Motivo por el que termino la reproduccion de una pista */
typedef enum { TRK_END = 0, TRK_NEXT = 1, TRK_PREV = 2 } TrackResult;

static uint32_t audio_block[128];     /* bloque PCM (512 B)                     */
static uint8_t  prefix[2048];         /* prefijo para escanear cabecera + meta  */
static uint32_t btn_last = 0xF;       /* estado previo de los botones (sueltos) */

/* Detecta el flanco "recien presionado" en KEY2/KEY3. */
static int poll_botones(void) {
    uint32_t now  = BUTTONS_DATA & 0xF;
    uint32_t just = (btn_last & ~now) & 0xF;   /* 1 (suelto) -> 0 (presionado) */
    btn_last = now;
    if (just & KEY_NEXT_MASK) return TRK_NEXT;
    if (just & KEY_PREV_MASK) return TRK_PREV;
    return -1;
}

/* Reproduce una pista; devuelve por que termino (fin natural o boton). */
static TrackResult reproducir_pista(const char* nombre, uint32_t track_num) {
    Fat32File f;
    WavHeader wh;
    char title[WAV_INFO_MAX], artist[WAV_INFO_MAX];
    uint32_t data_off = 0, data_size = 0;

    if (fat32_open(nombre, &f) != 0) {
        printf("[WARN] No se encontro '%s'.\n", nombre);
        return TRK_END;
    }
    int pn = fat32_read(&f, prefix, sizeof(prefix));
    if (pn < 44 ||
        wav_scan(prefix, (uint32_t)pn, &wh, &data_off, &data_size,
                 title, artist, WAV_INFO_MAX) != WAV_OK) {
        printf("[WARN] '%s' no es WAV PCM valido.\n", nombre);
        return TRK_END;
    }

    printf("\n=== %lu: %s (%s) ===\n", (unsigned long)track_num,
           title[0] ? title : nombre, artist);

    /* Anunciar pista al NIOS: el numero de pista viaja en el payload de
     * CMD_TRACK_START para que el NIOS muestre la pista correcta. */
    audio_bridge_init(&wh, track_num);
    audio_bridge_send_text(title, artist);
    audio_bridge_play();

    /* Reposicionar al inicio del audio PCM (saltar la cabecera) */
    fat32_open(nombre, &f);
    uint32_t to_skip = data_off;
    while (to_skip > 0) {
        wdt_pet();
        uint32_t n = (to_skip > 512U) ? 512U : to_skip;
        int r = fat32_read(&f, audio_block, n);
        if (r <= 0) break;
        to_skip -= (uint32_t)r;
    }

    /* Streaming NO bloqueante: revisa botones en cada vuelta. */
    uint32_t bloque = 0, restantes = data_size;
    while (restantes > 0) {
        wdt_pet();                     /* mantener vivo el HPS (watchdog) */
        int b = poll_botones();
        if (b == TRK_NEXT || b == TRK_PREV) {
            audio_bridge_track_end();
            return (TrackResult)b;
        }
        /* Enviar un bloque solo si hay lugar para sus 129 palabras (1 cmd + 128) */
        if (audio_bridge_fifo_free() < AUDIO_BRIDGE_BLOCK_WORDS) {
            continue;   /* FIFO lleno (o en pausa): re-chequear botones */
        }
        uint32_t pedir = (restantes >= 512U) ? 512U : restantes;
        memset(audio_block, 0, sizeof(audio_block));
        int n = fat32_read(&f, audio_block, pedir);
        if (n <= 0) break;
        audio_bridge_send_block(audio_block, bloque++);
        restantes -= (uint32_t)n;
    }

    audio_bridge_track_end();
    return TRK_END;   /* fin natural */
}

int main(void) {
    printf("\n=== REPRODUCTOR DE AUDIO (HPS BARE-METAL) ===\n");

    wdt_pet();
    sd_init();
    wdt_pet();
    if (fat32_mount() != 0) {
        printf("[ERROR] No se pudo montar la SD (FAT32).\n");
        return 1;
    }
    printf("[OK] SD montada.\n");
    fat32_list_root();

    /* Reproduccion continua con navegacion (REQ-01/REQ-02) */
    uint32_t cur = 0;
    while (1) {
        wdt_pet();
        TrackResult r = reproducir_pista(PLAYLIST[cur], cur + 1);
        if (r == TRK_PREV) {
            cur = (cur == 0) ? (PLAYLIST_LEN - 1) : (cur - 1);
        } else {                       /* TRK_NEXT o fin natural: avanzar */
            cur = (cur + 1) % PLAYLIST_LEN;
        }
    }
    return 0;
}
