# Lector FAT32 (solo lectura) — Bare-Metal HPS

Permite abrir las canciones de la SD **por nombre de archivo** en vez de por
sector crudo. Se apoya en el driver SD (`sd_read_block`) y corre en el ARM
bare-metal, igual que la prueba del driver.

## Archivos

| Archivo | Rol |
|---|---|
| `sw/fat32/fat32.c/.h` | Montaje, listado del root, apertura por nombre 8.3 y lectura secuencial. |
| `sw/fat_test_main.c` | Prueba: monta, lista el directorio raíz y abre una canción. |
| `sw/Makefile` (target `fat_test`) | Compila la prueba como app bare-metal. |

## API

```c
int  fat32_mount(void);                       // detecta MBR o superfloppy + BPB
int  fat32_open(const char* name, Fat32File* f); // "SONG1.WAV" (se convierte a 8.3)
int  fat32_read(Fat32File* f, void* dst, uint32_t len); // bytes leidos secuenciales
void fat32_list_root(void);                   // listado de depuracion
```

Uso típico para streaming de un WAV:
```c
Fat32File f;
fat32_mount();
fat32_open("SONG1.WAV", &f);
uint8_t header[44];
fat32_read(&f, header, 44);          // cabecera WAV
uint8_t block[512];
int n;
while ((n = fat32_read(&f, block, sizeof(block))) > 0) {
    /* enviar 'block' (n bytes de PCM) al FIFO de la FPGA */
}
```

## Alcance y limitaciones (intencionales)

- **Solo lectura**, sectores de 512 B.
- **Nombres cortos 8.3** (sin Long File Names). Si en Windows el archivo se llama
  `cancion-larga.wav`, guardalo con un nombre corto tipo `SONG1.WAV`.
- Solo el **directorio raíz** (no recorre subdirectorios). Poné las canciones en
  la raíz de la partición.
- Detecta automáticamente **MBR** (tarjeta con tabla de particiones, lo normal al
  formatear con Windows o el RPi Imager) o **superfloppy** (FAT sin particionar).

## Compilar y probar

```sh
cd sw
make fat_test          # -> fat_test_bare.bin
```
Cargá `fat_test_bare.bin` igual que la prueba del driver (ver
[`baremetal_app.md`](baremetal_app.md)): `fatload mmc 0:1 0x00100000
fat_test_bare.bin; go 0x00100000`. Por el UART deberías ver el listado del root
y los metadatos del WAV abierto.

> Preparar la SD: formateala FAT32 en Windows y copiá las canciones (con nombres
> 8.3) a la raíz. Ya no hace falta `dd` a sectores crudos.

## Verificación realizada

El módulo se probó en el host contra imágenes FAT32 **reales** (generadas con
`pyfatfs`), en dos layouts:
- **Superfloppy** (FAT sin particionar).
- **MBR + partición FAT32** en LBA 2048 (como una SD formateada normal).

En ambos: montaje correcto, listado del root, apertura por nombre (incluido en
minúsculas) y lectura completa de un archivo de 200 KB **byte-a-byte idéntica**
al original, recorriendo la cadena de clusters y cruzando límites de sector.

## Siguiente integración

Reemplazar en el reproductor (`sw/main.c`) la lectura por sector fijo
(`sd_read_block(n, ...)`) por `fat32_open` + `fat32_read`, para recorrer las 10+
canciones por nombre y enviar sus bloques al FIFO IPC de la FPGA.
