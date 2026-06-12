#!/bin/bash
# =============================================================================
# make_sd_image.sh   (Linux / WSL)
# Arma una imagen 'sdcard.img' LISTA PARA FLASHEAR (con RPi Imager, balenaEtcher
# o dd) para arranque bare-metal del HPS en la DE1-SoC.
#
# Layout:
#   p1  FAT32 (0x0C)  -> u-boot.img, reproductor_bare.bin, u-boot.scr, *.wav
#   p2  A2    (0xA2)  -> preloader-mkpimage.bin   (al offset 0 de la particion)
#
# El preloader (config FAT_SUPPORT) carga u-boot.img desde la FAT; U-Boot corre
# u-boot.scr que carga el reproductor y salta a el.  Sin teclear nada.
#
# REQUISITOS (en la maquina Linux/WSL):  sudo apt install mtools dosfstools
#
# USO:
#   ./make_sd_image.sh \
#        --preloader sw/preloader/preloader-mkpimage.bin \
#        --uboot     sw/preloader/uboot-socfpga/u-boot.img \
#        --app       sw/reproductor_bare.bin \
#        --scr       hw/scripts/u-boot.scr \
#        --songs     ./songs \
#        --out       sdcard.img
# =============================================================================
set -e

PRELOADER=""; UBOOT=""; APP=""; SCR=""; SONGS=""; OUT="sdcard.img"
FAT_SLACK_MB=128     # espacio extra en la FAT ademas de las canciones

while [ $# -gt 0 ]; do
    case "$1" in
        --preloader) PRELOADER="$2"; shift 2;;
        --uboot)     UBOOT="$2";     shift 2;;
        --app)       APP="$2";       shift 2;;
        --scr)       SCR="$2";       shift 2;;
        --songs)     SONGS="$2";     shift 2;;
        --out)       OUT="$2";       shift 2;;
        *) echo "Opcion desconocida: $1"; exit 1;;
    esac
done

# --- Verificaciones ---
for f in "$PRELOADER" "$UBOOT" "$APP"; do
    [ -f "$f" ] || { echo "ERROR: falta archivo requerido: '$f'"; exit 1; }
done
command -v mcopy   >/dev/null || { echo "ERROR: instala mtools (sudo apt install mtools)"; exit 1; }
command -v mkfs.vfat >/dev/null || { echo "ERROR: instala dosfstools"; exit 1; }

# --- Calcular tamaño de la FAT segun el peso de las canciones ---
SONGS_KB=0
if [ -n "$SONGS" ] && [ -d "$SONGS" ]; then
    SONGS_KB=$(du -sk "$SONGS" | cut -f1)
fi
FAT_MB=$(( SONGS_KB / 1024 + FAT_SLACK_MB ))
[ "$FAT_MB" -lt 256 ] && FAT_MB=256

# --- Geometria (sectores de 512 B) ---
SEC=512
P1_START=2048
P1_SECTORS=$(( FAT_MB * 1024 * 1024 / SEC ))
P2_START=$(( P1_START + P1_SECTORS ))
P2_SECTORS=$(( 1 * 1024 * 1024 / SEC ))           # 1 MB para el preloader
DISK_SECTORS=$(( P2_START + P2_SECTORS + 2048 ))

echo ">>> FAT32: ${FAT_MB} MB | imagen total: $(( DISK_SECTORS * SEC / 1024 / 1024 )) MB"

# --- 1) Construir la particion FAT32 en un archivo aparte ---
FAT_IMG=$(mktemp)
dd if=/dev/zero of="$FAT_IMG" bs=$SEC count="$P1_SECTORS" status=none
mkfs.vfat -F32 "$FAT_IMG" >/dev/null

echo ">>> Copiando archivos a la FAT..."
mcopy -i "$FAT_IMG" "$UBOOT" ::u-boot.img
mcopy -i "$FAT_IMG" "$APP"   ::reproductor_bare.bin
[ -n "$SCR" ] && [ -f "$SCR" ] && mcopy -i "$FAT_IMG" "$SCR" ::u-boot.scr
if [ -n "$SONGS" ] && [ -d "$SONGS" ]; then
    for w in "$SONGS"/*; do
        [ -f "$w" ] && mcopy -i "$FAT_IMG" "$w" "::$(basename "$w")"
    done
fi

# --- 2) Ensamblar el disco: MBR + FAT + A2(preloader) ---
echo ">>> Ensamblando '$OUT'..."
dd if=/dev/zero of="$OUT" bs=$SEC count="$DISK_SECTORS" status=none

python3 - "$OUT" "$P1_START" "$P1_SECTORS" "$P2_START" "$P2_SECTORS" <<'PY'
import sys, struct
out, p1s, p1n, p2s, p2n = sys.argv[1], *map(int, sys.argv[2:6])
mbr = bytearray(512)
def part(off, ptype, start, size):
    mbr[off+4] = ptype
    mbr[off+8:off+12]  = struct.pack('<I', start)
    mbr[off+12:off+16] = struct.pack('<I', size)
part(0x1BE, 0x0C, p1s, p1n)   # FAT32 LBA
part(0x1CE, 0xA2, p2s, p2n)   # Altera boot (preloader)
mbr[0x1FE] = 0x55; mbr[0x1FF] = 0xAA
with open(out, 'r+b') as f:
    f.seek(0); f.write(mbr)
print("    MBR escrito (p1 FAT32 @%d, p2 A2 @%d)" % (p1s, p2s))
PY

# Copiar la FAT formateada al offset de p1
dd if="$FAT_IMG" of="$OUT" bs=$SEC seek="$P1_START" conv=notrunc status=none
# Copiar el preloader al inicio de la particion A2 (p2)
dd if="$PRELOADER" of="$OUT" bs=$SEC seek="$P2_START" conv=notrunc status=none

rm -f "$FAT_IMG"
sync
echo ""
echo ">>> LISTO: $OUT"
echo "    Flashealo con RPi Imager (Use custom) / balenaEtcher / dd y enchufa la SD."
