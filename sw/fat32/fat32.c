#include "fat32.h"
#include "sd_driver.h"
#include <string.h>
#include <stdio.h>

/* =============================================================================
 * Estado del volumen montado
 * ========================================================================== */
static uint32_t s_part_lba;        /* primer sector de la partición (VBR)      */
static uint8_t  s_sec_per_clus;    /* sectores por cluster                     */
static uint32_t s_fat_begin_lba;   /* primer sector de la FAT                  */
static uint32_t s_clus_begin_lba;  /* sector del cluster 2 (inicio del área de datos) */
static uint32_t s_root_cluster;    /* cluster del directorio raíz             */
static int      s_mounted = 0;

/* Buffer de trabajo (FAT y entradas de directorio), alineado a 32 bits */
static uint32_t s_scratch[128];

#define SECTOR_SIZE   512u
#define FAT_EOC       0x0FFFFFF8u   /* >= esto = fin de cadena                 */

/* -------------------------------------------------------------------------
 * Helpers de lectura little-endian desde un buffer de bytes
 * ------------------------------------------------------------------------- */
static uint16_t rd16(const uint8_t* p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}
static uint32_t rd32(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void read_sector(uint32_t lba, uint32_t* dst128) {
    sd_read_block(lba, dst128);
}

/* LBA del primer sector de un cluster */
static uint32_t clus_to_lba(uint32_t clus) {
    return s_clus_begin_lba + (clus - 2u) * s_sec_per_clus;
}

/* Siguiente cluster en la cadena, consultando la FAT */
static uint32_t fat_next(uint32_t clus) {
    uint32_t fat_offset = clus * 4u;
    uint32_t fat_sector = s_fat_begin_lba + (fat_offset / SECTOR_SIZE);
    uint32_t ent        = fat_offset % SECTOR_SIZE;
    read_sector(fat_sector, s_scratch);
    return rd32((const uint8_t*)s_scratch + ent) & 0x0FFFFFFFu;
}

/* Convierte "SONG1.WAV" al formato 8.3 empaquetado de 11 bytes ("SONG1   WAV") */
static void to_83(const char* name, char out[11]) {
    int i = 0, o = 0;
    for (o = 0; o < 11; o++) out[o] = ' ';
    /* nombre base (hasta 8 chars, hasta el punto) */
    o = 0;
    while (name[i] && name[i] != '.' && o < 8) {
        char c = name[i++];
        if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
        out[o++] = c;
    }
    /* saltar el resto del nombre base si excede 8 chars */
    while (name[i] && name[i] != '.') i++;
    /* extensión */
    if (name[i] == '.') {
        i++;
        o = 8;
        while (name[i] && o < 11) {
            char c = name[i++];
            if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
            out[o++] = c;
        }
    }
}

/* -------------------------------------------------------------------------
 * Montaje
 * ------------------------------------------------------------------------- */
int fat32_mount(void) {
    uint8_t* sec = (uint8_t*)s_scratch;
    uint32_t vbr_lba = 0;

    s_mounted = 0;

    read_sector(0, s_scratch);
    printf("[FAT] sec0 leido; firma %02X %02X; sec[0]=%02X\n",
           sec[0x1FE], sec[0x1FF], sec[0]);
    if (rd16(sec + 0x1FE) != 0xAA55u) {
        return -1; /* sin firma de boot */
    }

    /* ¿El sector 0 ya es un BPB (superfloppy) o es un MBR con particiones? */
    int is_bpb = ((sec[0] == 0xEB) || (sec[0] == 0xE9)) && (rd16(sec + 0x0B) == SECTOR_SIZE);
    printf("[FAT] is_bpb=%d\n", is_bpb);

    if (is_bpb) {
        vbr_lba = 0;
    } else {
        /* Buscar la primera partición FAT32 (tipo 0x0B o 0x0C) en la tabla MBR */
        int found = 0;
        for (int i = 0; i < 4; i++) {
            const uint8_t* e = sec + 0x1BE + i * 16;
            uint8_t type = e[4];
            printf("[FAT] part%d: tipo=%02X lba=%lu\n",
                   i, type, (unsigned long)rd32(e + 8));
            if (type == 0x0B || type == 0x0C) {
                vbr_lba = rd32(e + 8);
                found = 1;
                break;
            }
        }
        if (!found) return -2;

        printf("[FAT] leyendo VBR en lba=%lu...\n", (unsigned long)vbr_lba);
        read_sector(vbr_lba, s_scratch);
        printf("[FAT] VBR leido; firma %02X %02X\n", sec[0x1FE], sec[0x1FF]);
        if (rd16(sec + 0x1FE) != 0xAA55u) return -3;
    }

    /* Parsear el BPB */
    if (rd16(sec + 0x0B) != SECTOR_SIZE) return -4;  /* solo sectores de 512 B */

    s_sec_per_clus = sec[0x0D];
    uint16_t rsvd  = rd16(sec + 0x0E);
    uint8_t  nfat  = sec[0x10];
    uint32_t fatsz = rd32(sec + 0x24);   /* FATSz32 */
    s_root_cluster = rd32(sec + 0x2C);   /* BPB_RootClus */

    printf("[FAT] spc=%u rsvd=%u nfat=%u fatsz=%lu root=%lu\n",
           s_sec_per_clus, rsvd, nfat,
           (unsigned long)fatsz, (unsigned long)s_root_cluster);

    if (s_sec_per_clus == 0 || nfat == 0 || fatsz == 0) return -5;

    s_part_lba       = vbr_lba;
    s_fat_begin_lba  = vbr_lba + rsvd;
    s_clus_begin_lba = vbr_lba + rsvd + (uint32_t)nfat * fatsz;
    s_mounted = 1;
    return 0;
}

/* -------------------------------------------------------------------------
 * Apertura por nombre 8.3 en el directorio raíz
 * ------------------------------------------------------------------------- */
int fat32_open(const char* name, Fat32File* f) {
    if (!s_mounted) return -1;

    char want[11];
    to_83(name, want);

    uint32_t clus = s_root_cluster;
    uint8_t* sec  = (uint8_t*)s_scratch;

    while (clus >= 2u && clus < FAT_EOC) {
        uint32_t lba = clus_to_lba(clus);
        for (uint32_t s = 0; s < s_sec_per_clus; s++) {
            read_sector(lba + s, s_scratch);
            for (uint32_t off = 0; off < SECTOR_SIZE; off += 32) {
                uint8_t* e = sec + off;
                if (e[0] == 0x00) return -2;        /* fin del directorio       */
                if (e[0] == 0xE5) continue;         /* entrada borrada          */
                uint8_t attr = e[0x0B];
                if (attr == 0x0F) continue;         /* fragmento LFN            */
                if (attr & 0x08) continue;          /* etiqueta de volumen      */
                if (memcmp(e, want, 11) == 0) {
                    uint32_t hi = rd16(e + 0x14);
                    uint32_t lo = rd16(e + 0x1A);
                    f->first_cluster  = (hi << 16) | lo;
                    f->size           = rd32(e + 0x1C);
                    f->bytes_left     = f->size;
                    f->cur_cluster    = f->first_cluster;
                    f->sector_in_clus = 0;
                    f->byte_in_sector = 0;
                    f->buf_valid      = 0;
                    return 0;
                }
            }
        }
        clus = fat_next(clus);
    }
    return -3; /* no encontrado */
}

/* -------------------------------------------------------------------------
 * Lectura secuencial
 * ------------------------------------------------------------------------- */
int fat32_read(Fat32File* f, void* dst, uint32_t len) {
    uint8_t* out  = (uint8_t*)dst;
    uint8_t* fbuf = (uint8_t*)f->buf;
    uint32_t total = 0;

    while (len > 0 && f->bytes_left > 0) {
        if (f->cur_cluster < 2u || f->cur_cluster >= FAT_EOC) break;

        if (!f->buf_valid) {
            uint32_t lba = clus_to_lba(f->cur_cluster) + f->sector_in_clus;
            read_sector(lba, f->buf);
            f->buf_valid = 1;
        }

        uint32_t avail = SECTOR_SIZE - f->byte_in_sector;
        uint32_t n = len;
        if (n > avail)          n = avail;
        if (n > f->bytes_left)  n = f->bytes_left;

        memcpy(out, fbuf + f->byte_in_sector, n);
        out            += n;
        total          += n;
        len            -= n;
        f->bytes_left  -= n;
        f->byte_in_sector += n;

        if (f->byte_in_sector >= SECTOR_SIZE) {
            f->byte_in_sector = 0;
            f->buf_valid      = 0;
            f->sector_in_clus++;
            if (f->sector_in_clus >= s_sec_per_clus) {
                f->sector_in_clus = 0;
                f->cur_cluster = fat_next(f->cur_cluster);
            }
        }
    }
    return (int)total;
}

/* -------------------------------------------------------------------------
 * Listado del directorio raíz (depuración)
 * ------------------------------------------------------------------------- */
void fat32_list_root(void) {
    if (!s_mounted) {
        printf("FAT32 no montado\n");
        return;
    }
    uint32_t clus = s_root_cluster;
    uint8_t* sec  = (uint8_t*)s_scratch;

    printf("Directorio raiz:\n");
    while (clus >= 2u && clus < FAT_EOC) {
        uint32_t lba = clus_to_lba(clus);
        for (uint32_t s = 0; s < s_sec_per_clus; s++) {
            read_sector(lba + s, s_scratch);
            for (uint32_t off = 0; off < SECTOR_SIZE; off += 32) {
                uint8_t* e = sec + off;
                if (e[0] == 0x00) return;
                if (e[0] == 0xE5) continue;
                uint8_t attr = e[0x0B];
                if (attr == 0x0F || (attr & 0x08)) continue;
                char nm[12];
                memcpy(nm, e, 11);
                nm[11] = '\0';
                uint32_t sz = rd32(e + 0x1C);
                printf("  %-11.11s  %10u B%s\n", nm, (unsigned)sz,
                       (attr & 0x10) ? "  <DIR>" : "");
            }
        }
        clus = fat_next(clus);
    }
}
