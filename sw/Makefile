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

# Prueba aislada del driver SD (sin FPGA, sin audio_bridge)
SD_TEST_ELF    = sd_test_hps.elf
SD_TEST_SRCS   = sd_test_main.c SD_Driver/sd_driver.c wav_parser/wav_parser.c
SD_TEST_CFLAGS = $(CFLAGS) --specs=nosys.specs -DSD_TEST_ONLY

sd_test: $(SD_TEST_SRCS)
	$(CC) $(SD_TEST_CFLAGS) $(SD_TEST_SRCS) -o $(SD_TEST_ELF)
	arm-none-eabi-objcopy -O binary $(SD_TEST_ELF) sd_test_hps.bin
	@echo ">>> Binario de prueba SD: sd_test_hps.bin"
	
# ---------------------------------------------------------------------------
# Prueba del driver SD como app BARE-METAL real para el HPS.
# Se enlaza con crt0 + linker script y salida por UART (printf funciona).
# Se carga desde U-Boot:  fatload mmc 0:1 0x00100000 sd_test_bare.bin; go 0x00100000
# Requiere el toolchain bare-metal de SoC EDS (arm-altera-eabi-*).
# ---------------------------------------------------------------------------
BARE_CC      = arm-altera-eabi-gcc
BARE_OBJCOPY = arm-altera-eabi-objcopy
BARE_ELF     = sd_test_bare.elf
BARE_BIN     = sd_test_bare.bin
BARE_LD      = startup/cyclone5_baremetal.ld
BARE_SRCS    = startup/crt0.S bsp/hps_uart.c bsp/syscalls.c \
               sd_test_main.c SD_Driver/sd_driver.c wav_parser/wav_parser.c
BARE_CFLAGS  = -mcpu=cortex-a9 -mfloat-abi=soft -marm -g -O2 -Wall \
               -ffreestanding -ISD_Driver -Iwav_parser -Ibsp
BARE_LDFLAGS = -nostartfiles -T $(BARE_LD)

bare: $(BARE_SRCS) $(BARE_LD)
	$(BARE_CC) $(BARE_CFLAGS) $(BARE_LDFLAGS) $(BARE_SRCS) -o $(BARE_ELF) -lc -lgcc
	$(BARE_OBJCOPY) -O binary $(BARE_ELF) $(BARE_BIN)
	@echo ">>> App bare-metal lista: $(BARE_BIN)  (entry 0x00100000)"
	@echo ">>> En U-Boot:  fatload mmc 0:1 0x00100000 $(BARE_BIN); go 0x00100000"

clean:
	rm -f *.o *.elf *.bin SD_Driver/*.o wav_parser/*.o audio_bridge/*.o \
	      startup/*.o bsp/*.o

.PHONY: qemu real clean sd_test bare