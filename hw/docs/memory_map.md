\# Mapa de Memoria del Sistema



\## Vista desde NIOS II



Direcciones asignadas manualmente para mantener una organización por

categoría (no usar Assign Base Addresses automático).



| Componente | Base | End | Tamaño | Categoría |

|------------|------|-----|--------|-----------|

| RAM (On-Chip) | `0x0000\_0000` | `0x0000\_FFFF` | 64 KB | Memoria |

| Nios II Debug | `0x0001\_0000` | `0x0001\_07FF` | 2 KB | CPU |

| System ID | `0x0002\_0000` | `0x0002\_0007` | 8 B | Identificación |

| JTAG UART | `0x0003\_0000` | `0x0003\_0007` | 8 B | Comunicación |

| Interval Timer | `0x0004\_0000` | `0x0004\_001F` | 32 B | Timing |

| Buttons PIO (IRQ) | `0x0005\_0000` | `0x0005\_000F` | 16 B | Entrada |

| Switches PIO | `0x0005\_1000` | `0x0005\_100F` | 16 B | Entrada |

| LEDs PIO | `0x0005\_2000` | `0x0005\_200F` | 16 B | Salida |



\## IRQs del NIOS II



| IRQ | Componente |

|-----|------------|

| 1 | JTAG UART |

| 2 | Interval Timer |

| 3 | Buttons PIO |



(Confirmar números reales con Platform Designer si fueron asignados diferente)



\## Registros de los PIOs



Cada PIO tiene 4 registros consecutivos de 32 bits:



| Offset | Nombre | R/W | Función |

|--------|--------|-----|---------|

| 0x00 | DATA | R/W | Valor actual de los pines |

| 0x04 | DIRECTION | R/W | Dirección (no usado en input-only / output-only) |

| 0x08 | INTERRUPTMASK | R/W | Máscara de interrupciones (solo buttons) |

| 0x0C | EDGECAPTURE | R/W | Captura de flancos (solo buttons) |



\## Registros del Interval Timer



| Offset | Nombre | R/W | Función |

|--------|--------|-----|---------|

| 0x00 | STATUS | R/W | bit 0: TO (timeout), bit 1: RUN |

| 0x04 | CONTROL | R/W | bit 0: ITO (IRQ enable), bit 1: CONT, bit 2: START, bit 3: STOP |

| 0x08 | PERIODL | R/W | Período (16 bits bajos) |

| 0x0C | PERIODH | R/W | Período (16 bits altos) |

| 0x10 | SNAPL | R/W | Snapshot (16 bits bajos) |

| 0x14 | SNAPH | R/W | Snapshot (16 bits altos) |



\## Vectors del NIOS



\- \*\*Reset vector\*\*: `0x0000\_0000` (en RAM)

\- \*\*Exception vector\*\*: `0x0000\_0020` (en RAM)



Última actualización: 2026-05-30

