CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = simulador_control

all: $(TARGET)

$(TARGET): simulador_control.c 
		$(CC) $(CFLAGS) simulador_control.c -o $(TARGET)

clean: 
		rm -f $(TARGET)