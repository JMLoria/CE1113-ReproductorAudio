# Arranque Bare-Metal del HPS: Preloader + Tarjeta SD

Guía para generar el **preloader** del HPS (Cyclone V) a partir del *handoff*
del proyecto y dejar la tarjeta SD lista para arrancar la aplicación bare-metal
del reproductor (`reproductor_hps.bin` / `sd_test_hps.bin`).

> **Contexto.** En la DE1-SoC la fuente de arranque del HPS (BSEL) está fijada
> por hardware a **SD/MMC**, así que no hay que mover switches de boot: basta con
> que la SD tenga el preloader en el lugar correcto. La Boot ROM del chip lee el
> preloader desde una **partición cruda con ID `0xA2`**, el preloader inicializa
> PLLs, SDRAM y pin-mux, y luego carga y salta a la siguiente imagen (tu app o
> U-Boot).

---

## 0. Requisitos

- **Intel SoC FPGA Embedded Development Suite (SoC EDS)** instalado (versión
  acorde a tu Quartus; el flujo `bsp-create-settings` aplica a SoC EDS 19.1+).
- El **handoff** generado por Quartus al compilar, ya presente en el repo:
  `hw/hps_isw_handoff/soc_system_hps_0/`. Si recompilás el hardware, este
  directorio se regenera y el preloader debe rehacerse.
- Una **microSD** y un **lector de SD** en la PC.
- Cable **USB-Blaster** (solo para programar la FPGA `.sof` por JTAG y para el
  puerto serie/JTAG-UART; no interviene en la grabación de la SD).
- Un **conversor USB-UART** (o el JTAG-UART) para ver los mensajes de arranque
  a **115200 8N1**.

Todos los comandos `bsp-*`, `mkpimage`, `mkimage` y `alt-boot-disk-util` viven
dentro del **SoC EDS Embedded Command Shell**:
- Windows: `…/intelFPGA/<ver>/embedded/Embedded_Command_Shell.bat`
- Linux: `source …/intelFPGA/<ver>/embedded/env.sh` (o `embedded_command_shell.sh`)

---

## 1. Generar y compilar el preloader

El preloader es un **U-Boot SPL** que el generador de BSP arma a la medida del
handoff (incluye la secuencia del calibrador de SDRAM, el pin-mux, los relojes,
etc.).

Desde la raíz del repo, dentro del Embedded Command Shell:

```sh
bsp-create-settings \
  --type spl \
  --bsp-dir       sw/preloader \
  --preloader-settings-dir "hw/hps_isw_handoff/soc_system_hps_0" \
  --settings      sw/preloader/settings.bsp

make -C sw/preloader
```

Al terminar tenés el archivo clave:

```
sw/preloader/preloader-mkpimage.bin
```

`mkpimage` ya le añadió la cabecera (y la réplica ×4) que exige la Boot ROM.

> Para ajustar opciones (soporte FAT, watchdog, tamaño de stack, etc.) abrí el
> editor gráfico con `bsp-editor`, cargá `sw/preloader/settings.bsp`, cambiá lo
> necesario, **Generate**, y volvé a `make`. El script
> `hw/scripts/build_preloader.sh` automatiza los dos comandos de arriba.

---

## 2. Preparar la aplicación bare-metal

> **Importante (paso siguiente, no incluido en esta parte).** La app actual
> (`sw/Makefile` target `real`) genera un `.bin` plano sin *linker script* ni
> *startup* (vectores, stack, dirección de carga). Para que el preloader la
> cargue y salte correctamente hay que enlazarla a la dirección que el preloader
> espera (típicamente **`0x00100040`** en SDRAM) con un linker script y un
> `crt0` mínimo de ARM Cortex-A9. El ejemplo de referencia
> [robertofem/CycloneVSoC-examples → SD-baremetal](https://github.com/robertofem/CycloneVSoC-examples/tree/master/SD-baremetal)
> trae exactamente ese linker script y Makefile para esta familia de placas.

Una vez que la app esté bien enlazada, se le pone la cabecera de la misma forma
que al preloader:

```sh
mkpimage -o sw/app-mkpimage.bin sw/sd_test_hps.bin
```

(`sd_test_hps.bin` es la prueba aislada del driver SD; cuando todo funcione se
reemplaza por `reproductor_hps.bin`.)

---

## 3. Rutas para que el preloader cargue la app

Hay dos formas de encadenar el preloader con tu app. Elegí una:

### Ruta A — App en la misma partición `0xA2` (más simple, sin U-Boot)

El preloader carga la "siguiente imagen" que esté grabada justo después de él en
la partición `0xA2`. Es la opción mínima para bare-metal: no necesitás U-Boot ni
partición FAT.

### Ruta B — U-Boot + carga desde FAT

El preloader carga **U-Boot**, y U-Boot carga tu `.bin` desde una partición FAT
(`fatload mmc 0:1 0x… app.bin; go 0x…`). Más pasos, pero te da consola, scripts
de arranque y flexibilidad. Útil si más adelante querés cargar también el `.rbf`
de la FPGA desde la SD. Requiere compilar U-Boot (`make uboot` en el BSP) y
ajustar el `bootcmd`.

Para el bring-up del driver SD, **usá la Ruta A**.

---

## 4. Particionar la SD y grabar el preloader

La SD necesita al menos una partición cruda tipo `0xA2` para el preloader.
Layout recomendado (sirve para Ruta A y, si después querés, Ruta B):

| Part. | Tipo            | Tamaño   | Contenido                                  |
|-------|-----------------|----------|--------------------------------------------|
| p1    | `0x0C` (FAT32)  | ~256 MB  | (opcional) U-Boot, `.rbf`, datos           |
| p3    | `0xA2` (raw)    | ~1 MB    | **preloader** (+ app en Ruta A)            |

> ⚠️ **Cuidado con el dispositivo.** Confirmá la letra/`/dev` correcto de la SD
> antes de escribir: equivocarse puede borrar otro disco. En Linux usá
> `lsblk`; en Windows mirá el número de disco en *Administración de discos*.

### 4a. Crear la partición A2 (una sola vez)

**Linux (fdisk):**
```sh
sudo fdisk /dev/sdX
#  n  → p → 3 → (primer sector por defecto) → +1M
#  t  → 3 → escribí  a2     (tipo de partición personalizado de Altera)
#  (opcional) n → p → 1 → ... → +256M ; t → 1 → c   (FAT32 LBA)
#  w  → escribir y salir
sudo mkfs.vfat -F32 /dev/sdX1     # solo si creaste la FAT
```

El script `hw/scripts/make_sd_baremetal.sh` automatiza esto con chequeos.

### 4b. Grabar el preloader (y la app en Ruta A)

**Con la utilidad de SoC EDS (recomendada, Windows o Linux):**
```sh
# Solo preloader:
alt-boot-disk-util -a write -p sw/preloader/preloader-mkpimage.bin -d <SD>

# Ruta A: preloader + app bare-metal juntos en la partición A2:
alt-boot-disk-util -a write -p sw/preloader/preloader-mkpimage.bin \
                            -b sw/app-mkpimage.bin -d <SD>
```
`<SD>` es la letra de unidad en Windows (ej. `E`) o el dispositivo en Linux.

**Alternativa Linux con `dd`** (escribe el preloader al inicio de la partición A2):
```sh
sudo dd if=sw/preloader/preloader-mkpimage.bin of=/dev/sdX3 bs=64k seek=0
sync
```

---

## 5. Programar la FPGA y conectar la consola

1. Con el **USB-Blaster**, programá el bitstream por JTAG (desarrollo):
   `quartus_pgm -m jtag -o "p;hw/Proyecto_II.sof"` (o desde el Programmer).
2. Conectá el **UART a 115200 8N1** (PuTTY / `screen /dev/ttyUSB0 115200`).
3. Insertá la SD y encendé/resetea la placa.

---

## 6. Qué deberías ver (verificación)

En la consola serie, el preloader imprime algo como:

```
U-Boot SPL 20xx.xx ...
...
SDRAM : Calibrating ...
SDRAM calibration was successful
...
```

- Si llega a **"SDRAM calibration was successful"**, el preloader corre y la
  SDRAM quedó inicializada: el arranque del HPS funciona.
- En **Ruta A**, a continuación debería saltar a tu app y deberías ver la salida
  de `sd_test_main.c` (init de la SD + parseo del WAV del sector 0).
- Si **no** imprime nada: revisá que el preloader esté en una partición tipo
  `0xA2`, que la SD esté bien insertada, y el cableado/baud del UART.

---

## 7. Resumen del orden de tareas

1. `bsp-create-settings … && make -C sw/preloader` → `preloader-mkpimage.bin` ✅ (esta parte)
2. Linker script + crt0 para la app bare-metal (siguiente parte) → `.bin` enlazado a `0x00100040`
3. `mkpimage` a la app → `app-mkpimage.bin`
4. Crear partición A2 en la SD
5. `alt-boot-disk-util -p preloader -b app -d <SD>`
6. Programar FPGA por JTAG, abrir UART, arrancar y verificar

---

## Fuentes

- [Generating and Compiling the Preloader — RocketBoards.org](https://www.rocketboards.org/foswiki/Documentation/AVGSRDPreloader)
- [Cyclone V: Preloader and Bootloader workflow (SoC EDS 19.1+) — RocketBoards Forum](https://forum.rocketboards.org/t/cyclone-v-preloader-and-bootloader-the-new-workflow-since-soc-eds-standard-version-19-1/2316)
- [AN 709: HPS SoC Boot Guide — Cyclone V SoC Development Kit](https://manuals.plus/m/9a87a119316d4d8eae77921ce07df6fd80a13043909e3f5a7ce17d488bc8c1c8)
- [Booting from SD/MMC – Custom Partition (A2) — Intel docs](https://www.intel.com/content/www/us/en/docs/programmable/683265/current/booting-from-sd-mmc-custom-partition.html)
- [Cyclone V SoC Preloader will not run bare metal app — Intel Community](https://community.intel.com/t5/Programmable-Devices/Cyclone-V-SoC-Preloader-will-not-run-bare-metal-app/m-p/239388)
- [robertofem/CycloneVSoC-examples → SD-baremetal (linker script + Makefile de referencia)](https://github.com/robertofem/CycloneVSoC-examples/tree/master/SD-baremetal)
