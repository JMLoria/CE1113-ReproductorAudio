# Análsis y Justificación de Co-Diseño Hardware/Software

Este documento detalla los criterios técnicos y arquitectónicos utilizados para realizar la partición de tareas entre el tejido lógico de la FPGA (Hardware) y los procesadores del sistema (Software) para el desarrollo del Reproductor de Audio Digital. 

El objetivo primordial de esta distribución es optimizar el rendimiento del sistema, garantizar el determinismo en el procesamiento de señales de audio y minimizar la latencia de respuesta ante la interacción del usuario.

## Resumen de la Partición de Componentes

La distribución de los módulos funcionales se definió bajo la siguiente arquitectura híbrida:
- **Implementado en Hardware (FPGA/IP Cores):**
    - Filtros de audion digitales
    - Interfaz de visualización
- **Implementado en Software (Porcesadores ARM HPS y Nios II):**
    - Control de almacenamiento
    - Control temporal
    - Control de reproducción
 
## Justificación de Componentes en Hardware

### Filtros de audio digitales
La escogencia de los filtros por Hardware se decidió porque resulta mucho más determinista y adecuado para tiempo real, permitiendo cumplir con las restricciones estrictas de baja latencia del sistema. Conforme van llegando las muestras de audio, estas se pueden ir procesando y filtrando de manera asíncrona mediante un circuito de silicio dedicado exclusivamente a esta tarea matemática, operando en paralelo al resto de componentes.

En el caso de haberlo implementado por Software, el procesador Nios II tendría que estar constantemente interrumpiendo su lazo o consumiendo millones de instrucciones de CPU para atender las operaciones de filtrado (desplazamientos, sumas y saturación) por cada muestra consecutiva. Esto mermaría la eficiencia general y elevaría drásticamente el riesgo de retrasos acústicos (underflow), considerando que la CPU ya debe llevar la carga asíncrona de las interrupciones de usuario y la lógica de control de la reproducción.

### Interfaz de visualización
El decodificador y multiplexor para los Displays de 7 segmentos se delegó al hardware mediante la creación de un bloque IP personalizado en el tejido de la FPGA. El refresco constante y la excitación física de los pines de los displays requieren una temporización rígida a frecuencias de kHz para evitar el parpadeo visual (flicker).

Mapear esta lógica en hardware libera por completo a la CPU de gestionar retardos y escaneos de pines. El procesador únicamente escribe un dato plano en un registro mapeado en memoria cuando ocurre un cambio real en el tiempo, y el circuito de hardware se encarga de sostener y decodificar la señal visual de manera autónoma.

## Justificación de Componentes en Software

### Control de almacenamiento
La gestión de la tarjeta SD, que incluye la inicialización del protocolo físico SPI/SDIO, el montaje del sistema de archivos (como FAT32) y el desempaquetado de la estructura binaria de las cabeceras de los archivos de audio ```.wav```, es una tarea masiva, altamente secuencial y variable.

Diseñar un circuito en hardware que maneje sistemas de archivos requiere una cantidad prohibitiva de compuertas lógicas y es sumamente inflexible ante cambios en los archivos. Por lo tanto, se delegó al procesador ARM (HPS) bajo un entorno Linux Embebido, un medio ideal y robusto para interactuar con sistemas de almacenamiento masivo utilizando llamadas de software estándar.

### Control temporal
La lógica encargada de llevar el conteo de reproducción en minutos y segundos se implementó por software mediante una Rutina de Servicio de Interrupción (ISR) ligada a un temporizador de hardware de 50 MHz.

El hardware del Interval Timer provee la precisión de reloj exacta (generando un pulso cada 1 segundo real), pero la aritmética de conversión (cálculo de residuos, divisiones automáticas para cascada de minutos y formateo a cadenas legibles) se resuelve mediante un algoritmo de software directo. Esto ofrece un balance perfecto entre la precisión absoluta del reloj de arena físico y la flexibilidad de cómputo del software.

### Control de reproducción
El motor principal que gobierna las transiciones del reproductor (Play, Pausa, Stop, saltos circulares de canciones, control de límites y el filtrado del rebote mecánico de los botones) se maneja por software a través de un patrón arquitectónico de Máquina de Estados Finitos (FSM).

Este comportamiento es intrínsecamente algorítmico e interactivo. Diseñarlo en software corriendo en el procesador embebido Nios II permite un código altamente ordenado, modular, robusto frente a ruidos eléctricos mediante máscaras temporales, y sumamente ligero (ocupando un espacio mínimo en la memoria RAM interna de la FPGA).

---

Este enfoque de co-diseño asegura que las capacidades de procesamiento paralelo de la FPGA se aprovechen en flujos de datos concurrentes y de alta velocidad, mientras que las CPUs manejan las tareas complejas de administración y control secuencial de manera eficiente.
