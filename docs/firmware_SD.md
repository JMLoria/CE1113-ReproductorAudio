# Documentación Técnica: Arranque Bare-Metal y Preparación de Tarjeta SD para HPS (Cyclone V)

---

## 1. Requisitos Previos

* **Software:** Intel SoC FPGA Embedded Development Suite (SoC EDS) instalado (acorde a la versión de Quartus).
* **Hardware:** Placa DE1-SoC, una tarjeta microSD y un lector de tarjetas SD en la PC.
* **Archivos:** El *handoff* generado por Quartus al compilar, ubicado en `hw/hps_isw_handoff/soc_system_hps_0/`.
* **Conexiones:** Cable USB-Blaster para programar la FPGA por JTAG y un conversor USB-UART configurado a **115200 8N1** para visualizar la consola.

---

## 2. Construcción de los Artefactos

Todos los comandos de esta sección deben ejecutarse dentro del **SoC EDS Embedded Command Shell**. 

### 2.1 Preloader
El preloader inicializa los PLLs, la SDRAM y el pin-mux del HPS. 
* Ejecuta `bash hw/scripts/build_preloader.sh` desde la raíz del repositorio.
* Esto automatiza los comandos `bsp-create-settings` y `make`, generando el archivo `sw/preloader/preloader-mkpimage.bin`.

### 2.2 U-Boot
El U-Boot será cargado por el preloader.
* Antes de compilar, abre `bsp-editor` y habilita `FAT_SUPPORT` junto con `FAT_LOAD_PAYLOAD_NAME = u-boot.img`.
* Ejecuta `make -C sw/preloader uboot` para generar el archivo `u-boot.img`.

### 2.3 Aplicación Bare-Metal (Reproductor)
* Edita el archivo `sw/player_main.c` y configura en la variable `PLAYLIST` los nombres de las canciones respetando el formato 8.3 (ej. `SONG1.WAV`).
* Ejecuta `cd sw && make player && cd ..` para compilar la aplicación, obteniendo `sw/reproductor_bare.bin`.

### 2.4 Script de Auto-Arranque
* Para automatizar el arranque desde U-Boot, ejecuta: `mkimage -A arm -T script -C none -d hw/scripts/boot.txt hw/scripts/u-boot.scr`.

---

## 3. Preparación y Grabado de la Tarjeta SD

La tarjeta SD debe contener una partición tipo FAT32 (`0x0C`) para U-Boot, la aplicación y las canciones, y una partición cruda tipo Altera boot (`0xA2`) para el preloader. Puedes elegir entre dos métodos.

### Método A: Imagen Flasheable en Linux/WSL (Recomendado)
Este método genera una imagen que se graba de una sola vez.
* Instalar las dependencias necesarias: `sudo apt install mtools dosfstools`.
* Ejecutar el script de creación de imagen: `hw/scripts/make_sd_image.sh --preloader sw/preloader/preloader-mkpimage.bin --uboot sw/preloader/uboot-socfpga/u-boot.img --app sw/reproductor_bare.bin --scr hw/scripts/u-boot.scr --songs ./songs --out sdcard.img`.
* Grabar el archivo resultante `sdcard.img` en tu tarjeta microSD utilizando herramientas como RPi Imager, balenaEtcher o `dd`.

### Método B: Preparación Manual en Windows
Si no se dispone de un entorno Linux, se utiliza `diskpart` nativo de Windows.
* Abrir la consola como Administrador y ejecuta `diskpart` para limpiar la tarjeta SD (`clean`).
* Crear la partición 1 primaria con formato FAT32 y asígnale una letra (ej. `F`).
* Crear la partición 2 primaria (tamaño 1 MB) y asígnale el ID `a2` para definirla como partición de arranque de Altera.
* En el SoC EDS Command Shell, ejecutar `alt-boot-disk-util -a write -p sw/preloader/preloader-mkpimage.bin -b sw/preloader/uboot-socfpga/u-boot.img -d F` para grabar el preloader y U-Boot en la partición cruda.
* Copiar los archivos `reproductor_bare.bin`, `u-boot.scr` y las canciones `.WAV` directamente a la unidad FAT32 montada.

---

## 4. Configuración de Auto-Arranque (Primer Uso)

Para que U-Boot inicie la aplicación automáticamente al encender el sistema:
* Durante el primer arranque, interrumpe la cuenta regresiva en la terminal serie para acceder al prompt de U-Boot.
* Ejecutar: `setenv bootcmd 'fatload mmc 0:1 0x00100000 reproductor_bare.bin; go 0x00100000'`.
* Guardar la configuración con `saveenv` y reinicia con `reset`.

---

## 5. Carga Externa: FPGA y Procesador Nios II

La tarjeta SD solo maneja el arranque del HPS. La configuración del hardware (FPGA) y el firmware de control (Nios II) deben cargarse de manera externa a través de la conexión JTAG.

**Importante:** La FPGA y el procesador Nios deben programarse **antes** de iniciar el HPS para que puedan consumir los datos del FIFO del reproductor y evitar un bloqueo por *backpressure*.

* Programa el bitstream de la FPGA: `quartus_pgm -m jtag -o "p;hw/Proyecto_II.sof"`.
* Descarga el firmware del Nios: `nios2-download -g sw/nios/app/<tu_app>.elf`.

---
