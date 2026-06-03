# =============================================================================
# Makefile — wav_parser test para QEMU versatilepb (Cortex-A9)
#
# Requisitos:
#   - arm-none-eabi-gcc   (paquete: gcc-arm-none-eabi en Debian/Ubuntu)
#   - qemu-system-arm     (paquete: qemu-system-arm)
#
# Uso rápido:
#   make          → compila firmware.elf
#   make run      → compila y ejecuta en QEMU (Ctrl-A X para salir)
#   make clean    → elimina artefactos
#   make check    → compila, ejecuta y captura el exit code
# =============================================================================

# --------------------------------------------------------------------------
# Toolchain
# --------------------------------------------------------------------------
CROSS   := arm-none-eabi
CC      := $(CROSS)-gcc
LD      := $(CROSS)-gcc
OBJCOPY := $(CROSS)-objcopy
SIZE    := $(CROSS)-size

# --------------------------------------------------------------------------
# Directorios y fuentes
# --------------------------------------------------------------------------
SRCS    := main.c wav_parser.c
OBJS    := $(SRCS:.c=.o)
TARGET  := firmware.elf

# --------------------------------------------------------------------------
# Flags de compilación
# --------------------------------------------------------------------------
# -mcpu=cortex-a9     : arquitectura objetivo (igual a la opción -cpu de QEMU)
# -marm               : genera instrucciones ARM de 32 bits (no Thumb)
# -mfloat-abi=soft    : sin FPU hardware en versatilepb
# -ffreestanding      : no asume librería C estándar de host
# -specs=rdimon.specs : habilita semihosting (printf → QEMU stdout)
# --specs=nosys.specs ya viene incluido por rdimon; no hace falta duplicar
# -g                  : símbolos de debug (útil con gdb + QEMU -gdb tcp::1234)
CFLAGS  := \
    -mcpu=cortex-a9   \
    -marm             \
    -mfloat-abi=soft  \
    -ffreestanding    \
    -Wall             \
    -Wextra           \
    -g                \
    -O0

LDFLAGS := \
    -mcpu=cortex-a9   \
    -marm             \
    -mfloat-abi=soft  \
    -specs=rdimon.specs \
    -lc               \
    -lrdimon

# --------------------------------------------------------------------------
# QEMU
# --------------------------------------------------------------------------
QEMU        := qemu-system-arm
QEMU_MACHINE:= versatilepb
QEMU_CPU    := cortex-a9
QEMU_MEM    := 128M
QEMU_FLAGS  := \
    -M $(QEMU_MACHINE) \
    -cpu $(QEMU_CPU)   \
    -m $(QEMU_MEM)     \
    -nographic         \
    -semihosting       \
    -audio none        \
    -kernel $(TARGET)


.PHONY: all run clean check size

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
	$(SIZE) $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Dependencias de headers (regeneración automática si cambia wav_parser.h)
main.o:       main.c wav_parser.h
wav_parser.o: wav_parser.c wav_parser.h

# Ejecutar en QEMU (interactivo; Ctrl-A X para salir)
run: $(TARGET)
	$(QEMU) $(QEMU_FLAGS)

# Ejecutar y verificar exit code (útil en CI / scripts)
# QEMU devuelve el exit code del guest via semihosting sys_exit.
check: $(TARGET)
	$(QEMU) $(QEMU_FLAGS); \
	RET=$$?; \
	if [ $$RET -eq 0 ]; then \
	    echo ""; \
	    echo ">>> Todos los tests pasaron (exit 0)"; \
	else \
	    echo ""; \
	    echo ">>> FALLÓ (exit $$RET)"; \
	    exit $$RET; \
	fi

size: $(TARGET)
	$(SIZE) $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)