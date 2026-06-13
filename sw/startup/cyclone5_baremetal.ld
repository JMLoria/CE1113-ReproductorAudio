/* =============================================================================
 * cyclone5_baremetal.ld — Linker script para la app bare-metal del HPS.
 *
 * La aplicación se enlaza para ejecutarse desde la SDRAM en 0x00100000 (1 MB),
 * que es la dirección a la que U-Boot la carga y a la que se salta con 'go'.
 * Queda por debajo de donde U-Boot se reubica, así que no hay colisión.
 * ========================================================================== */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

__stack_size = 0x8000;    /* 32 KB de stack */
__heap_size  = 0x40000;   /* 256 KB de heap (para malloc/printf de newlib) */

MEMORY
{
    SDRAM (rwx) : ORIGIN = 0x00100000, LENGTH = 0x01000000   /* ventana de 16 MB */
}

SECTIONS
{
    . = ORIGIN(SDRAM);

    /* _start tiene que quedar al inicio del binario (entry de 'go') */
    .start : { KEEP(*(.start)) } > SDRAM

    .text : {
        *(.text*)
        *(.gnu.linkonce.t.*)
    } > SDRAM

    .rodata : {
        *(.rodata*)
        *(.gnu.linkonce.r.*)
    } > SDRAM

    . = ALIGN(8);
    .data : {
        *(.data*)
        *(.gnu.linkonce.d.*)
    } > SDRAM

    . = ALIGN(8);
    .bss (NOLOAD) : {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    } > SDRAM

    /* Heap a continuación de .bss */
    . = ALIGN(8);
    __heap_start = .;
    . = . + __heap_size;
    __heap_end = .;

    /* Stack arriba del heap (crece hacia abajo desde __stack_top) */
    . = . + __stack_size;
    . = ALIGN(8);
    __stack_top = .;
}
