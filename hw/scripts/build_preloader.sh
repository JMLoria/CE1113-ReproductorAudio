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
# Asegurar que se use el 'tar' de Cygwin de SoC EDS y NO el de Windows
# (C:\Windows\System32\tar.exe), que no entiende rutas /cygdrive/... y rompe el
# untar de u-boot ("Error opening archive: Failed to open ...").
if [ -n "$SOCEDS_DEST_ROOT" ]; then
    CYGBIN="$(cygpath -u "$SOCEDS_DEST_ROOT" 2>/dev/null || echo "$SOCEDS_DEST_ROOT")/host_tools/cygwin/bin"
    [ -d "$CYGBIN" ] && export PATH="$CYGBIN:$PATH"
fi

# OJO: el 'make' de SoC EDS es nativo de Windows y no entiende rutas Cygwin
# (/cygdrive/c/...). Por eso hay que 'cd' al directorio (el cd del shell sí las
# resuelve) y correr make ahí, en vez de 'make -C <ruta-cygwin>'.
( cd "$BSP_DIR" && make )

echo ""
echo ">>> Listo:"
echo "    $BSP_DIR/preloader-mkpimage.bin"
echo ""
echo "    Siguiente: crear la partición A2 y grabarlo con"
echo "    hw/scripts/make_sd_baremetal.sh  (o alt-boot-disk-util)."
