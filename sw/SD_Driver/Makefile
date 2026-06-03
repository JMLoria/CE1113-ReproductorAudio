CC = arm-none-eabi-gcc
CFLAGS = -mcpu=cortex-a9 -mfloat-abi=soft -g -O0

all:
	$(CC) $(CFLAGS) -c sd_driver.c -o sd_driver.o
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(CC) $(CFLAGS) -o reproductor.elf main.o sd_driver.o