# Bring-up en la FPGA — Compilar y probar el sistema completo

Guía paso a paso para compilar las tres partes y correr el reproductor integrado
en la DE1-SoC. Todos los comandos asumen que estás en la **raíz del repo**.

Arquitectura del arranque:
```
JTAG (USB-Blaster):  FPGA (.sof)  +  firmware NIOS (.elf)
SD:                  preloader -> U-Boot -> reproductor_bare.bin (HPS)
```

---

## 1. Compilar las tres partes

### 1.1 App del HPS (reproductor)
En el **SoC EDS Command Shell**:
```sh
cd sw
make player
cd ..
```
Genera `sw/reproductor_bare.bin`.

### 1.2 Firmware del NIOS (control: FIFO/decode, botones, hex, VGA)
En el **Nios II Command Shell** (o Nios II SBT for Eclipse):
```sh
cd sw/nios/app
make
cd ../../..
```
Genera `sw/nios/app/control_app.elf`.
> Si `make` falla por rutas del BSP, regenerá el BSP: `cd ../control_app_bsp && ./create-this-bsp` y reintentá.
> Recordá que `Makefile` del Nios ahora compila `main.c vga.c vga_ui.c`.

### 1.3 FPGA (.sof)
**No hace falta recompilar**: no cambiamos hardware (qsys/RTL), solo software.
Usás el `hw/Proyecto_II.sof` que ya tenés. (Si igual querés regenerarlo: abrí el
proyecto en Quartus y *Processing → Start Compilation*.)

---

## 2. Actualizar la SD

Como `reproductor_bare.bin` cambió, copiá el nuevo a la partición FAT (`G:`):
```sh
cp sw/reproductor_bare.bin /cygdrive/g/
```
(El preloader, U-Boot, `u-boot.scr` y las canciones siguen igual.)

---

## 3. Conexiones físicas

- **USB-Blaster** (JTAG) al puerto del FPGA.
- **USB-UART** de la DE1-SoC a la PC (terminal serie a **115200 8N1**).
- **microSD** insertada.
- **Salida de audio** (line-out / auriculares) al códec.
- **Monitor VGA** conectado.

---

## 4. Cargar y correr (el ORDEN importa)

> Clave: el HPS accede a la FPGA (FIFO, botones) por el puente. **La FPGA tiene
> que estar configurada ANTES de que el HPS la use.** Por eso se programa la
> FPGA y el NIOS primero, y después se resetea el HPS.

1. **Encendé** la placa.

2. **Programá la FPGA** por JTAG:
   ```sh
   quartus_pgm -m jtag -o "p;hw/Proyecto_II.sof"
   ```
   (o con el **Quartus Programmer** GUI.)

3. **Descargá y corré el firmware del NIOS** por JTAG:
   ```sh
   nios2-download -g sw/nios/app/control_app.elf
   ```
   (Opcional: en otra ventana, `nios2-terminal` para ver los `printf`/stdio del NIOS.)

4. **Reseteá el HPS** con el botón de **reset del HPS** (HPS_WARM_RST) para que
   arranque desde la SD **con la FPGA ya configurada**. Bootea:
   `preloader -> U-Boot -> reproductor_bare.bin`.

5. **Abrí el terminal serie** (PuTTY / `screen /dev/ttyUSB0 115200`) para ver la
   salida del HPS (montaje de SD, pista actual, etc.).

### 4.1 Auto-arranque (solo la PRIMera vez)
En el prompt de U-Boot (apretá una tecla para interrumpir el autoboot):
```
setenv bootcmd 'fatload mmc 0:1 0x00100000 reproductor_bare.bin; go 0x00100000'
saveenv
reset
```
Desde ahí arranca el reproductor solo. (Fallback siempre válido: teclear esas dos
líneas `fatload`/`go` a mano.)

---

## 5. Verificación (checklist de la demo)

- [ ] Por el **serie del HPS**: "SD montada", listado de canciones, y por cada
      pista el título/artista.
- [ ] **Sale audio** por el códec.
- [ ] **SW[1:0]** cambian el filtro: 00 bypass, 01 low-pass, 10 high-pass, 11 bass.
- [ ] **VGA** muestra título, artista, duración y tiempo MM:SS.
- [ ] **Hex** muestra el tiempo MM:SS.
- [ ] **KEY0** Play/Pausa, **KEY1** Stop, **KEY2** Siguiente, **KEY3** Anterior
      (Siguiente/Anterior cambian la canción real).

---

## 6. Problemas comunes

| Síntoma | Causa probable / solución |
|---|---|
| No bootea el HPS (serie mudo) | Preloader no está en la partición A2, o la SD no está bien insertada. Revisá el orden de partición (FAT = partición 1). |
| Bootea U-Boot pero no corre la app | Faltó el `setenv bootcmd; saveenv`, o `reproductor_bare.bin` no está en la FAT. Probá el `fatload`/`go` a mano. |
| Audio con ruido/glitches | El NIOS no decodifica el protocolo (firmware viejo). Rebuildeá y bajá el `control_app.elf` nuevo. |
| No suena nada | El NIOS no está corriendo (¿se descargó el `.elf`?) o el HPS arrancó antes que la FPGA: reseteá el HPS (paso 4). |
| Botones/VGA no responden | FPGA no configurada cuando arrancó el HPS: reprogramá FPGA + NIOS y reseteá el HPS. |
| Filtros no cambian | Dominio de R_DSP: verificá SW[1:0] y la cadena AudioFilter. |

---

## 7. Resumen del flujo

```
make player            (HPS)   -> reproductor_bare.bin -> copiar a SD (G:)
make (sw/nios/app)     (NIOS)  -> control_app.elf
hw/Proyecto_II.sof     (FPGA)  (ya existe)

En la placa:
  quartus_pgm  (.sof)  ->  nios2-download (.elf)  ->  reset HPS  ->  serie 115200
```
