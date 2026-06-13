/* =============================================================================
 * crt0.S — Startup mínimo bare-metal para ARM Cortex-A9 (HPS del Cyclone V).
 *
 * La aplicación se carga desde U-Boot con:  fatload ... 0x00100000 app.bin
 * y se salta con:  go 0x00100000
 * Por eso _start DEBE ser lo primero del binario (sección .start, colocada al
 * inicio por el linker script).
 *
 * Tareas: modo SVC con IRQ/FIQ enmascaradas, deshabilitar MMU y caches (para
 * acceso directo y ordenado a los registros MMIO del SD/MMC y la UART),
 * inicializar el stack, limpiar .bss y llamar a main().
 * ========================================================================== */
    .section .start, "ax"
    .global _start
    .arm
_start:
    /* Modo Supervisor (SVC), IRQ (I) y FIQ (F) enmascaradas */
    msr     cpsr_c, #0xD3

    /* Deshabilitar MMU, D-cache e I-cache en el System Control Register (SCTLR) */
    mrc     p15, 0, r0, c1, c0, 0
    bic     r0, r0, #(1 << 0)      /* M  : MMU off               */
    bic     r0, r0, #(1 << 2)      /* C  : Data cache off        */
    bic     r0, r0, #(1 << 12)     /* I  : Instruction cache off */
    mcr     p15, 0, r0, c1, c0, 0
    dsb
    isb

    /* Stack pointer (definido por el linker script) */
    ldr     sp, =__stack_top

    /* Limpiar la sección .bss a cero */
    ldr     r0, =__bss_start
    ldr     r1, =__bss_end
    mov     r2, #0
1:  cmp     r0, r1
    strlo   r2, [r0], #4
    blo     1b

    /* Saltar a la aplicación C */
    bl      main

    /* main() no debería retornar; si lo hace, quedarse aquí */
hang:
    b       hang

    .size _start, . - _start
