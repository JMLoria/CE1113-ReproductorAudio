# Compilar y Cargar la App Bare-Metal del HPS

Cómo construir la prueba del driver SD como aplicación bare-metal real para el
HPS y cargarla en la placa. Complementa a [`preloader_sd.md`](preloader_sd.md)
(que deja el preloader en la SD).

---

## 1. Qué se agregó

Para correr en el ARM sin sistema operativo, la app necesita arranque propio y
una salida de texto. Se añadieron:

| Archivo | Rol |
|---|---|
| `sw/startup/crt0.S` | Entry `_start`: modo SVC, apaga MMU/caches, stack, limpia `.bss`, llama a `main()`. |
| `sw/startup/cyclone5_baremetal.ld` | Enlaza la app en SDRAM a **`0x00100000`**; define heap y stack. |
| `sw/bsp/hps_uart.c/.h` | Transmite por la **UART0 del HPS** (`0xFFC02000`), reusando la config de U-Boot (115200 8N1). |
| `sw/bsp/syscalls.c` | Redirige `printf → _write → UART` y da `_sbrk` (heap) a newlib. |
| `sw/Makefile` (target `bare`) | Compila todo con el toolchain bare-metal de SoC EDS. |

Con esto, los `printf` de `sd_test_main.c` y del driver salen por el puerto
serie de la placa.

---

## 2. Compilar

Dentro del **SoC EDS Command Shell** (tiene `arm-altera-eabi-gcc` en el PATH):

```sh
cd sw
make bare
```

Salida:
```
sd_test_bare.elf
sd_test_bare.bin     <- esto es lo que se carga en el HPS
```

> Si tu toolchain se llama distinto (`arm-none-eabi-gcc`), editá `BARE_CC` y
> `BARE_OBJCOPY` arriba del target `bare` en `sw/Makefile`.

---

## 3. Cargar y ejecutar (vía U-Boot)

Usamos U-Boot como cargador porque permite elegir la dirección explícitamente,
sin depender de detalles internos del preloader. Flujo:

**Preparar la SD** (además del preloader de `preloader_sd.md`):
- Que el preloader esté configurado para cargar U-Boot (en `bsp-editor`:
  `FAT_SUPPORT = 1`, `FAT_LOAD_PAYLOAD_NAME = u-boot.img`), y compilá U-Boot:
  ```sh
  make -C sw/preloader uboot
  ```
- Copiá a la **partición FAT (p1)** de la SD:
  - `u-boot.img`
  - `sd_test_bare.bin`

**En la placa:**
1. Programá la FPGA por JTAG con el USB-Blaster (`quartus_pgm -m jtag -o "p;hw/Proyecto_II.sof"`).
2. Abrí el UART a **115200 8N1**.
3. Encendé/resetea. Cuando aparezca el prompt de U-Boot, **interrumpí el autoboot**
   (tecla cualquiera) y ejecutá:
   ```
   fatload mmc 0:1 0x00100000 sd_test_bare.bin
   go 0x00100000
   ```
4. Deberías ver la salida de `sd_test_main.c`: la secuencia de init de la SD y el
   parseo del encabezado WAV del sector 0.

> **Alternativa sin U-Boot (más avanzada):** cargar el `.elf` por JTAG con el
> *Intel FPGA Monitor Program* (target ARM Cortex-A9, con el preloader ya
> corriendo). Útil para depurar paso a paso, pero requiere más configuración.

---

## 4. Poner un WAV donde el driver lo lea

El driver lee **sectores crudos por LBA** (no hay sistema de archivos todavía).
`sd_test_main.c` lee el **sector 0**, así que para la prueba hay que escribir un
WAV justo ahí.

> ⚠️ El sector 0 es normalmente la tabla de particiones (MBR). Para una prueba
> rápida y aislada podés usar una **segunda SD** dedicada solo a datos, o más
> seguro: cambiá en `sd_test_main.c` el `sd_read_block(0, ...)` por un LBA
> dentro de una partición de datos y escribí el WAV ahí.

Escribir un WAV a un LBA conocido (Linux):
```sh
# Ejemplo: grabar cancion.wav a partir del sector 40960 (20 MB) de la SD de datos
sudo dd if=cancion.wav of=/dev/sdX bs=512 seek=40960 conv=fsync
```
Y en `sd_test_main.c`, leer ese mismo sector:
```c
sd_read_block(40960, buffer);
```

Esto valida el camino completo: preloader → app → driver SD → lectura real →
parser WAV. Una vez verificado, el siguiente paso (futuro) es un **lector FAT32
mínimo** para abrir las canciones por nombre en lugar de por sector fijo.

---

## 5. Checklist de esta etapa

- [ ] `make bare` produce `sd_test_bare.bin` sin errores.
- [ ] SD con preloader (A2) + U-Boot + `sd_test_bare.bin` (FAT).
- [ ] FPGA programada por JTAG, UART a 115200.
- [ ] `fatload … 0x00100000 … ; go 0x00100000` arranca la app.
- [ ] Se ve por serie la init de la SD y los datos del sector leído.
