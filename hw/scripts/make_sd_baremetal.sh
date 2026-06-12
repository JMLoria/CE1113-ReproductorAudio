#!/bin/bash
# =============================================================================
# make_sd_baremetal.sh   (Linux)
# Prepara una microSD para arranque bare-metal del HPS Cyclone V (DE1-SoC):
#   - crea una partición cruda tipo 0xA2 para el preloader (+ app)
#   - (opcional) crea una partición FAT32
#   - graba el preloader (y, en Ruta A, la app) en la partición A2
#
# USO:
#   sudo ./make_sd_baremetal.sh /dev/sdX  preloader-mkpimage.bin  [app-mkpimage.bin]
#
# ⚠️  /dev/sdX debe ser la TARJETA SD. Verificá con 'lsblk' ANTES de correr.
#     Un error acá puede BORRAR otro disco.
# =============================================================================
set -e

DEV="$1"
PRELOADER="$2"
APP="$3"            # opcional (Ruta A: preloader + app en la partición A2)

if [ -z "$DEV" ] || [ -z "$PRELOADER" ]; then
    echo "Uso: sudo $0 /dev/sdX preloader-mkpimage.bin [app-mkpimage.bin]"
    exit 1
fi
if [ ! -b "$DEV" ]; then
    echo "ERROR: '$DEV' no es un dispositivo de bloque."
    exit 1
fi
if [ ! -f "$PRELOADER" ]; then
    echo "ERROR: no existe el preloader '$PRELOADER'."
    exit 1
fi

echo "Dispositivo destino:"
lsblk -o NAME,SIZE,MODEL,MOUNTPOINT "$DEV" || true
echo ""
read -r -p "¿Seguro que '$DEV' es la SD? Se BORRARÁ por completo. Escribí 'SI': " ok
[ "$ok" = "SI" ] || { echo "Cancelado."; exit 1; }

# Desmontar cualquier partición montada
for p in "${DEV}"*; do
    mountpoint -q "$p" 2>/dev/null && sudo umount "$p" || true
done

echo ">>> Creando tabla de particiones (p1 FAT32 256M, p3 A2 1M)..."
# Usamos sfdisk para un layout determinista.
sudo sfdisk "$DEV" <<EOF
label: dos
unit: sectors

${DEV}1 : start=2048,   size=524288, type=c
${DEV}3 : start=526336, size=2048,   type=a2
EOF

sudo partprobe "$DEV" || true
sleep 1

echo ">>> Formateando la FAT32 (p1)..."
sudo mkfs.vfat -F32 "${DEV}1"

echo ">>> Grabando el preloader en la partición A2 (p3)..."
sudo dd if="$PRELOADER" of="${DEV}3" bs=64k seek=0 conv=fsync

if [ -n "$APP" ] && [ -f "$APP" ]; then
    # Ruta A: la app va inmediatamente después del preloader en la A2.
    # 'alt-boot-disk-util' lo hace de forma más segura; si lo tenés, preferilo:
    #   alt-boot-disk-util -a write -p "$PRELOADER" -b "$APP" -d "$DEV"
    echo ">>> Grabando la app bare-metal después del preloader (offset 256k)..."
    sudo dd if="$APP" of="${DEV}3" bs=64k seek=4 conv=fsync
fi

sync
echo ""
echo ">>> SD lista. Insertala en la DE1-SoC, programá la FPGA por JTAG y"
echo "    abrí el UART a 115200 8N1 para ver el arranque del preloader."
