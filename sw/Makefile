# =============================================================================
# Makefile — SoC Audio Reproductor (Bare Metal ARM Cortex-A9)
# Targets:
#   make qemu    -> Compila con mock hardware y corre en QEMU
#   make real    -> Compila para Cyclone V HPS (sin QEMU_TEST)
#   make clean   -> Limpia artefactos
# =============================================================================

CC      = arm-none-eabi-gcc
CFLAGS  = -mcpu=cortex-a9 -mfloat-abi=soft -g -O0 -Wall -ISD_Driver -Iwav_parser -Iaudio_bridge
SRCS    = main.c SD_Driver/sd_driver.c wav_parser/wav_parser.c audio_bridge/audio_bridge.c 

# Configuración para Simulación en QEMU
QEMU_ELF     = reproductor_qemu.elf
QEMU_CFLAGS  = $(CFLAGS) -DQEMU_TEST --specs=rdimon.specs
QEMU_LDFLAGS = -lrdimon  
QEMU_CMD     = qemu-system-arm \
               -machine versatilepb \
               -cpu cortex-a9 \
               -nographic \
               -semihosting \
               -kernel $(QEMU_ELF)

qemu: $(SRCS)
	$(CC) $(QEMU_CFLAGS) $(SRCS) -o $(QEMU_ELF) $(QEMU_LDFLAGS)
	@echo ""
	@echo ">>> Compilacion OK. Ejecutando en QEMU..."
	@echo "    (Presiona Ctrl+A y luego X para salir de QEMU)"
	@echo ""
	$(QEMU_CMD)

# Configuración para Hardware Real Cyclone V HPS
REAL_ELF    = reproductor_hps.elf
REAL_CFLAGS = $(CFLAGS) --specs=nosys.specs

real: $(SRCS)
	$(CC) $(REAL_CFLAGS) $(SRCS) -o $(REAL_ELF)
	arm-none-eabi-objcopy -O binary $(REAL_ELF) reproductor_hps.bin
	@echo ">>> ELF generado: $(REAL_ELF)"
	@echo ">>> Binario para tarjeta SD: reproductor_hps.bin"

clean:
	rm -f *.o *.elf SD_Driver/*.o wav_parser/*.o audio_bridge/*.o

.PHONY: qemu real clean