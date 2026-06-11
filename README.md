# CE1113-ReproductorAudio

# Proyecto II: Co-diseño Hardware/Software de SoC con Hardware Personalizado para Reproductor de Audio Avanzado

## 1. Información General
* **Institución:** Instituto Tecnológico de Costa Rica
* **Escuela:** Ingeniería en Computadores
* **Curso:** CE-1113 Sistemas Empotrados
* **Profesor:** Dr.-Ing. Jeferson González Gómez
* **Fecha de entrega:** 12 de junio, 2026
* **Equipo:** Roy Chavarria, Jose Loria, Jose Solano, Noemi Vargas

## 2. Descripción del Proyecto

El proyecto consiste en el co-diseño hardware/software de un SoC avanzado para un reproductor de audio portátil en una tarjeta Altera DE-SoC1, utilizando una metodología de diseño a nivel de sistema (System Level Design). El equipo aplicará un flujo de trabajo moderno que incluye el modelado de sistemas heterogéneos y el análisis de trade-offs para particionar funciones entre el procesador HPS y la lógica FPGA. Se implementará un sistema Bare Metal con acceso directo al hardware, evitando bibliotecas de abstracción (HAL) para desarrollar filtros digitales en tiempo real y controladores de almacenamiento personalizados. El enfoque técnico integra la gestión de metadatos y la lógica de reproducción con aceleradores de hardware dedicados, justificando cada decisión arquitectural basándose en el rendimiento y la latencia. 

## 3. Estructura Organizativa (Roles)

| Código | Rol | Responsable | Responsabilidades |
| :--- | :--- | :--- | :--- |
| R_SoC | Hardware SoC | Jose Solano  | Diseño en Platform Designer, configuración del HPS, integración del bus Avalon y control de displays 7-seg. |
| R_DSP | Hardware DSP | Noemi Vargas | Diseño de los 3 filtros digitales en RTL (VHDL/Verilog), gestión del IP de Audio y aceleración de hardware. |
| R_Level | Software Low-Level | Roy Chavarria | Desarrollo del driver de la SD (SPI/SDIO), sistema de archivos Bare Metal y parseo de metadatos WAV. |
| R_Control | Software de Control | Jose Loria | Gestión de interrupciones, lógica de la máquina de estados de reproducción, control temporal y coordinación de UI. |

## 4. Matriz de Requerimientos

| Código | Categoría | Requerimiento | Descripción Detallada | Responsable |
| :--- | :--- | :--- | :--- | :--- |
| **REQ-01** | Usuario | Reproducción de audio | Soporte de al menos 10 canciones en formato WAV (16-bit) con frecuencias de 8kHz, 16kHz y 44.1kHz. | **R_Level** |
| **REQ-02** | Usuario | Navegación de pistas | Control secuencial (Anterior/Siguiente) y reproducción automática de la siguiente canción al finalizar la actual. | **R_Control** |
| **REQ-03** | Usuario | Selección de filtros | Aplicación en tiempo real de Filtro 1, Filtro 2 o Filtro 3 mediante el uso de switches físicos. | **R_DSP** |
| **REQ-04** | Funcional | Módulo HW Personalizado | Diseño de un componente con registros de control, estado y datos mapeados a memoria mediante bus Avalon-MM. | **R_SoC** |
| **REQ-05** | Funcional | Procesamiento DSP | Implementación de filtros digitales directamente en la lógica de la FPGA para procesamiento en tiempo real. | **R_DSP** |
| **REQ-06** | Funcional | Interfaz de Audio | El manejo del flujo de audio hacia el Codec debe ser realizado exclusivamente por la FPGA mediante un IP dedicado en Platform Designer. | **R_DSP** |
| **REQ-07** | Funcional | Control de Almacenamiento | Implementación del control de lectura de la tarjeta SD a través del controlador nativo de hardware en el ARM (HPS). | **R_Level** |
| **REQ-08** | Funcional | Parseo de Metadatos | Extracción manual (en el ARM HPS) del título, artista, álbum y duración total desde la estructura del archivo WAV. | **R_Level** |
| **REQ-09** | Funcional | Visualización UI | Visualización de metadatos y estado de filtros en una interfaz externa (VGA/LCD/TFT) conectada a la FPGA. | **R_SoC** |
| **REQ-10** | Funcional | Control Temporal | Manejo del tiempo transcurrido (MM:SS) mediante software Bare Metal en el NIOS II, actualizando los registros físicos del hardware. | **R_Control** |
| **REQ-11** | Funcional | Gestión de Control | Lógica de reproducción (Play, Pausa, Stop) operada mediante rutinas de interrupción (ISR) para los botones físicos en el NIOS II. | **R_Control** |
| **REQ-12** | No Funcional | Tiempo de Inicio | El sistema completo (subida de Linux en HPS y firmware en NIOS II) debe estar operativo en un tiempo menor a 10 segundos. | **R_SoC** |
| **REQ-13** | No Funcional | Latencia de Control | Tiempo de respuesta del NIOS II ante las interrupciones de los botones físicos debe ser inferior a 200ms. | **R_Control** |
| **REQ-14** | No Funcional | Latencia de Filtro | El cambio dinámico entre filtros digitales en la FPGA debe realizarse con una latencia imperceptible inferior a 100ms. | **R_DSP** |
| **REQ-15** | No Funcional | Estabilidad | Reproducción continua sin distorsiones audibles por vaciado de buffers, ni bloqueos o descalces en la comunicación entre procesadores. | **R_DSP / R_Control** |
| **REQ-16** | Restricción | Desarrollo Bare Metal | Programación de bajo nivel en el NIOS II utilizando punteros directos a memoria mapeada, evitando métodos previstos en el HAL. | **R_Control** |
| **REQ-17** | Funcional | Comunicación IPC | Implementación de un canal de comunicación inter-procesador (Dual-Port RAM o FIFO) en hardware para transferir datos de audio de ARM a NIOS II. | **R_SoC** |

## 5 Cronogroma de Proyecto

El presente cronograma detalla la hoja de ruta estratégica para el desarrollo del Reproductor de Audio SoC, estructurado en un ciclo de cuatro semanas que abarca desde el 16 de mayo hasta el 12 de junio de 2026. La planificación se fundamenta en una metodología de co-diseño moderno, priorizando la integración temprana de la interfaz hardware/software para mitigar riesgos técnicos. 

| Semana / Fase | Tarea a Iniciar | Hito / Entrega (Tarea Finalizada) | Fecha Entrega | Responsable |
|---|---|---|---|---|
| Semana 1 (16-22 May) — Cimientos y SoC | Configuración de la plataforma en Platform Designer y definición del bus Avalon. | Mapa de Memoria Documentado: Direcciones base y registros listos para el equipo de software. | 22-May | R_SoC |
|  | Desarrollo del Driver SD (comunicación SPI/SDIO) en modo Bare Metal. | Driver SD Funcional: Capacidad de lectura de bloques crudos de la tarjeta. | 22-May | R_Level |
| Semana 2 (23-29 May) — Procesamiento | Implementación del IP de Audio y diseño RTL de los filtros en la FPGA. | IP de Audio Integrado: Salida de sonido básica (Bypass) verificada. | 29-May | R_DSP |
|  | Desarrollo del sistema de archivos y parseo de encabezados WAV. | Módulo de Datos: Extracción exitosa de metadatos y envío de muestras al buffer. | 29-May | R_Level |
|  | Configuración del sistema de interrupciones para los botones físicos. | Control de Interrupciones: Respuesta del sistema ante pulsadores físicos. | 29-May | R_Control |
| Semana 3 (30 May - 5 Jun) — Interfaz y DSP | Refinamiento de Filtro 1, 2 y 3 con lógica de selección por switches. | Aceleradores DSP Listos: Filtros funcionales con latencia < 100ms. | 05-Jun | R_DSP |
|  | Desarrollo de la UI (VGA/LCD) y control de los displays 7-segmentos. | Interfaz Visual Operativa: Visualización de metadatos y tiempo MM:SS. | 05-Jun | R_SoC |
|  | Lógica de la máquina de estados para Play, Pausa y cambio de pista. | Lógica de Reproducción: Control total del flujo de canciones. | 05-Jun | R_Control |
| Semana 4 (6-12 Jun) — Integración | Pruebas de integración Hardware/Software y flujo de datos continuo. | Sistema Estable: Reproducción de 10+ canciones sin errores ni distorsión. | 10-Jun | Todo el equipo |
|  | Optimización de tiempos (inicio < 10s) y latencia de respuesta (< 200ms). | Cierre Técnico: Cumplimiento de requisitos no funcionales y estabilidad. | 11-Jun | Todo el equipo |
|  | Finalización de documentación y repositorio Git (Git Flow). | Entrega Final: Repositorio completo con README, diagramas y evidencias. | 12-Jun | Todo el equipo |

Se establece que las fechas de inicio y entrega de tareas internas poseen un carácter dinámico y adaptativo, lo cual establece que las fechas podrán ser ajustadas a conveniencia del grupo. Esta flexibilidad permite al equipo priorizar etapas de depuración compleja o adelantar integraciones según el progreso de los módulos individuales, garantizando siempre el cumplimiento de la entrega final y la estabilidad del sistema.

## 6. Flujo de Trabajo en Git

### 6.1. Estrategia de Ramas
* `main`: Rama de producción (código estable).
* `develop`: Rama de integración para el trabajo diario.
* `feat/nombre-tarea`: Desarrollo de nuevas funciones.
* `fix/nombre-error`: Corrección de fallos.

### 6.2. Convención de Commits (Conventional Commits)
Formato: `tipo(alcance): descripción`
* `feat`: Nueva funcionalidad.
* `fix`: Corrección de errores.
* `docs`: Cambios en README o documentación de atributos (DI/AC).
* `refactor`: Mejoras en código existente sin cambiar comportamiento.
* `test`: Creación o actualización de pruebas.

### 6.3. Pull Requests
* Todo PR debe estar vinculado a un **Issue**.
* Se requiere al menos **una aprobación** de un compañero para realizar el merge a `develop`.
