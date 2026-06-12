#!/bin/sh
# =============================================================================
# build_preloader.sh
# Genera y compila el preloader (U-Boot SPL) del HPS Cyclone V desde el handoff.
#
# EJECUTAR DENTRO DEL "SoC EDS Embedded Command Shell":
#   Windows: .../intelFPGA/<ver>/embedded/Embedded_Command_Shell.bat
#   Linux:   source .../intelFPGA/<ver>/embedded/env.sh
#
# Salida: sw/preloader/preloader-mkpimage.bin
# =============================================================================
set -e

# Raíz del repo (este script vive en hw/scripts/)
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
HANDOFF="$REPO_ROOT/hw/hps_isw_handoff/soc_system_hps_0"
BSP_DIR="$REPO_ROOT/sw/preloader"

# Verificaciones previas
if ! command -v bsp-create-settings >/dev/null 2>&1; then
    echo "ERROR: 'bsp-create-settings' no está en el PATH."
    echo "       Abrí el SoC EDS Embedded Command Shell y volvé a correr este script."
    exit 1
fi

if [ ! -d "$HANDOFF" ]; then
    echo "ERROR: no se encontró el handoff del HPS:"
    echo "       $HANDOFF"
    echo "       Compilá el proyecto en Quartus para regenerarlo."
    exit 1
fi

mkdir -p "$BSP_DIR"

echo ">>> [1/2] Generando BSP del preloader desde el handoff..."
bsp-create-settings \
    --type spl \
    --bsp-dir       "$BSP_DIR" \
    --preloader-settings-dir "$HANDOFF" \
    --settings      "$BSP_DIR/settings.bsp"

echo ">>> [2/2] Compilando preloader..."
make -C "$BSP_DIR"

echo ""
echo ">>> Listo:"
echo "    $BSP_DIR/preloader-mkpimage.bin"
echo ""
echo "    Siguiente: crear la partición A2 y grabarlo con"
echo "    hw/scripts/make_sd_baremetal.sh  (o alt-boot-disk-util)."
