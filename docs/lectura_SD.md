# Instrucciones de Compilación y Configuración del Entorno

## Dependencias
Para compilar y simular la parte de lectura del SD, es necesario contar con:
* **Toolchain:** `arm-none-eabi-gcc` para la compilación *bare-metal* de la arquitectura ARM.
* **Emulador:** `qemu-system-arm` para la simulación del procesador y pruebas de software sin requerir la placa física.

## Comandos de Compilación
Si se quiere probar esta seccion por si misma se pueden usar los comandos:
* `make qemu`: Compila el firmware utilizando hardware simulado (`mock_hardware`) y lanza automáticamente la simulación en la consola.
* `make real`: Genera el binario crudo `reproductor_hps.bin`, optimizado y listo para ser cargado en la partición FAT32 de la tarjeta SD de arranque del Cyclone V.
* `make sd_test`: Genera un binario de prueba aislada (`sd_test_hps.bin`) para verificar exclusivamente la lectura física del sector LBA 0 de la tarjeta SD, sin depender de la FPGA.
* `make clean`: Limpia el entorno de trabajo eliminando ejecutables y artefactos de compilación (`.o`, `.elf`, `.bi).

# Arquitectura de Software

El programa que se ejecuta en el procesador ARM Cortex-A9 (HPS) está modularizado en tres componentes principales:

* **`SD_Driver`:** Controlador *bare-metal* encargado de manejar la inicialización física de la tarjeta SD mediante secuencias de comandos (CMD0, CMD8, CMD17, etc.) y la extracción de bloques lógicos de 512 bytes hacia la memoria RAM.
* **`wav_parser`:** Módulo de procesamiento que decodifica la cabecera RIFF/WAVE del archivo de audio. Utiliza estructuras de datos empaquetadas (`__attribute__((packed))`) para extraer dinámicamente metadatos esenciales, como el *Sample Rate*, la cantidad de canales y el tamaño exacto de la carga útil de audio.
* **`audio_bridge`:** Capa de abstracción encargada de gestionar la máquina de estados de la reproducción. Actúa como el puente de comunicación, controlando el flujo y la inyección de los bloques de audio procesados hacia el hardware personalizado de la FPGA.


# Documentación de la Interfaz y Mapas de Memoria (Co-diseño)

La comunicación entre el procesador ARM (HPS) y el hardware de audio en la FPGA se realiza a través del bus Avalon, siguiendo este esquema de interacción:

## Transferencia de Datos
El ARM se encarga de enviar las muestras de audio en formato PCM estéreo. Para maximizar la eficiencia del bus, se empaquetan dos muestras de 16 bits (Canal Derecho y Canal Izquierdo) en una única palabra de 32 bits. Esta palabra empaquetada se escribe de forma secuencial directamente en la dirección base de la memoria FIFO: `0xFF200000`.

## Sincronización (Handshake)
Para evitar el desbordamiento de la FIFO (*buffer overflow*) debido a la alta velocidad del procesador frente a la tasa de consumo de hardware, el sistema implementa un mecanismo de sincronización mediante *polling*. El ARM lee constantemente el registro de control y estado (CSR) ubicado en la dirección `0xFF210000`. Si se detecta que la bandera `FIFO_FULL` está activada por el hardware, el procesador pausa la inyección de datos temporalmente hasta que se libere espacio en el búfer.
