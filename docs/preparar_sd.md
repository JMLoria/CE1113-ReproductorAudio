# Preparar la tarjeta SD para la defensa

Objetivo: dejar la microSD lista de una vez, de modo que el día de la defensa
**solo la enchufás y arranca el reproductor**.

> **Alcance.** La SD se ocupa del **lado HPS** (arranque del ARM + el reproductor
> + las canciones). La **FPGA (`.sof`)** y el **firmware del Nios (`.elf`)** se
> cargan por **JTAG con el USB-Blaster** (rápido y confiable). Ver §4.

---

## 0. Qué va a contener la SD

| Partición | Tipo | Contenido |
|---|---|---|
| **p1** | FAT32 (`0x0C`) | `u-boot.img`, `reproductor_bare.bin`, `u-boot.scr`, **canciones `*.WAV`** (nombres 8.3) |
| **p2** | Altera boot (`0xA2`) | `preloader-mkpimage.bin` |

Cadena de arranque: **Boot ROM → preloader (p2) → U-Boot (p1) → reproductor (p1)**.

---

## 1. Construir los artefactos (en el SoC EDS Command Shell)

Desde la raíz del repo:

```sh
# 1. Preloader (configura PLLs, SDRAM, pin-mux del HPS)
bash hw/scripts/build_preloader.sh         # -> sw/preloader/preloader-mkpimage.bin

# 2. U-Boot (lo carga el preloader). En bsp-editor habilitá FAT_SUPPORT y
#    FAT_LOAD_PAYLOAD_NAME = u-boot.img ANTES de compilar.
make -C sw/preloader uboot                 # -> u-boot.img

# 3. El reproductor (app del HPS)
cd sw && make player && cd ..              # -> sw/reproductor_bare.bin

# 4. Script de auto-arranque de U-Boot
mkimage -A arm -T script -C none -d hw/scripts/boot.txt hw/scripts/u-boot.scr
```

Quedan estos 4 archivos: `preloader-mkpimage.bin`, `u-boot.img`,
`reproductor_bare.bin`, `u-boot.scr`.

> Antes de seguir, **editá `sw/player_main.c`** y poné en `PLAYLIST` los nombres
> reales de tus canciones (8.3, ej. `SONG1.WAV`), y recompilá con `make player`.

---

## 2. Escribir la SD — elegí un método

### Método A — Imagen flasheable (recomendado, "enchufar y listo")

En una máquina **Linux o WSL** (`sudo apt install mtools dosfstools`):

```sh
hw/scripts/make_sd_image.sh \
    --preloader sw/preloader/preloader-mkpimage.bin \
    --uboot     sw/preloader/uboot-socfpga/u-boot.img \
    --app       sw/reproductor_bare.bin \
    --scr       hw/scripts/u-boot.scr \
    --songs     ./canciones \
    --out       sdcard.img
```

Genera `sdcard.img` con todo adentro (particiones, preloader, U-Boot, app y
canciones). Grabalo con **RPi Imager** ("Use custom" → `sdcard.img`),
balenaEtcher o `dd`. Listo: una imagen, un flasheo.

### Método B — Directo en Windows (SoC EDS)

1. **Crear la partición A2.** Windows no crea particiones tipo `0xA2`; usá una
   máquina Linux/WSL una sola vez: `fdisk /dev/sdX` → partición 1 FAT32 (tipo
   `c`) + partición 3 tipo `a2`; luego `mkfs.vfat -F32 /dev/sdX1`.
2. **Grabar el preloader** (y U-Boot) en la A2, desde el SoC EDS Command Shell:
   ```sh
   alt-boot-disk-util -a write -p preloader-mkpimage.bin -b u-boot.img -d <unidad>
   ```
   (`<unidad>` = letra de la SD en Windows, ej. `E`.)
3. **Copiar a la FAT** (arrastrar en el Explorador): `reproductor_bare.bin`,
   `u-boot.scr` y las canciones `*.WAV`.

---

## 3. Auto-arranque (configuración de una sola vez)

Para que U-Boot corra el reproductor **sin teclear nada**, hacelo una vez:

**Opción recomendada — fijar `bootcmd` y guardarlo.** Al primer arranque, cuando
veas la cuenta regresiva de U-Boot, apretá una tecla para entrar al prompt y:

```
setenv bootcmd 'fatload mmc 0:1 0x00100000 reproductor_bare.bin; go 0x00100000'
saveenv
reset
```

Desde ahí, cada vez que enchufes la SD arranca solo. (Si tu U-Boot ya corre
`u-boot.scr` automáticamente, ese paso ni hace falta.)

**Fallback que SIEMPRE funciona** (si el auto-arranque falla en la defensa):
entrá al prompt de U-Boot y escribí las dos líneas a mano:

```
fatload mmc 0:1 0x00100000 reproductor_bare.bin
go 0x00100000
```

---

## 4. Lo que NO va en la SD: FPGA + Nios (por JTAG)

El reproductor del HPS llena el FIFO, pero **alguien tiene que consumirlo**: la
FPGA y el Nios. Cargalos por JTAG con el USB-Blaster antes de arrancar el HPS:

```sh
# 1. Bitstream de la FPGA
quartus_pgm -m jtag -o "p;hw/Proyecto_II.sof"

# 2. Firmware del Nios (control: filtros, hex, botones)
nios2-download -g sw/nios/app/<tu_app>.elf
```

> **Orden importante:** programá FPGA y Nios **primero**; recién después arrancá
> el HPS desde la SD. Si el HPS arranca y el Nios no consume el FIFO, el stream
> se bloquea esperando (backpressure).

---

## 5. Checklist de defensa

1. [ ] microSD con la imagen grabada, insertada en la placa.
2. [ ] USB-Blaster conectado; cable USB del **UART** conectado (terminal 115200 8N1).
3. [ ] Programar FPGA: `quartus_pgm -m jtag -o "p;hw/Proyecto_II.sof"`.
4. [ ] Descargar firmware del Nios por JTAG.
5. [ ] Encender/resetear el HPS → arranca el reproductor desde la SD.
6. [ ] Verificar: sale audio por el códec, los **switches** cambian el filtro,
       el **hex** muestra el tiempo, los **botones** controlan la reproducción.

---

## 6. Opcional (avanzado): FPGA también desde la SD

Si querés que ni el `.sof` necesite JTAG: convertí el bitstream a `.rbf`
(`quartus_cpf -c Proyecto_II.sof Proyecto_II.rbf`), copialo a la FAT, y en
`u-boot.scr` agregá antes del `go` la carga al FPGA Manager
(`fatload mmc 0:1 0x2000000 Proyecto_II.rbf; fpga load 0 0x2000000 <tamaño>;
bridge enable`). Requiere ajustar los **MSEL** de la placa para configuración
HPS→FPGA. El firmware del Nios igual habría que embeberlo en la on-chip RAM del
`.sof`. Es más trabajo y conviene coordinarlo con el equipo; para la defensa,
JTAG (§4) es lo más seguro.
