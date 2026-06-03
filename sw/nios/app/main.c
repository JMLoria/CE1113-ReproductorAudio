/*
 * main.c
 */

#include "memory_map.h"

// Lee el registro de control interno de la CPU (reg_num)
#define NIOS2_READ_CTL_REG(reg_num) __builtin_rdctl(reg_num)

// Escribe un valor en el registr de control interno de la CPU (reg_num)
#define NIOS2_WRITE_CTL_REG(reg_num, value) __builtin_wrctl(reg_num, value)

// Definicion de numeros de registros internos segun la arquitectura Nios II
#define NIOS2_CTL_STATUS 0   // ctl0: Registro de estado (contiene bit PIE)
#define NIOS2_CTL_IENABLE 3  // ctl3: Registro de habilitacion de IRQs de la CPU
#define NIOS2_CTL_IPENDING 4 // ctl4: Registro que muestra que IRQs estan pendientes

// Mascara para habilitar interrupciones globales (Processor Interrupt Enable - PIE)
#define NIOS2_STATUS_PIE_MASK (1 << 0)

// Definicion de estados del reproductor
typedef enum {
	STATE_STOP,
	STATE_PLAY,
	STATE_PAUSE
} PlayerState_t;

// Variables globales de estado compartidas (Volatiles para evitar optimizaciones del compilador)
volatile uint32_t bandera_boton_presionado = 0;
volatile uint32_t boton_detectado = 0;
volatile uint32_t bandera_timer_un_segundo = 0;

// Variables de control global del reproductor
volatile PlayerState_t estado_actual = STATE_STOP;
volatile uint32_t cancion_actual = 1;
volatile uint32_t TOTAL_CANCIONES = 10;

// Variables para el control temporal
volatile uint32_t tiempo_segundos = 0;
volatile uint32_t minutos = 0;
volatile uint32_t segundos = 0;

// Variable global para almacenar el "tiempo del ultimo boton presionado"
volatile uint32_t tiempo_ultimo_click = 0;

// Esta sera nuestra funcion ISR dedicada a los botones fisicos
void boton_isr(void) {
	// 1. Leer que boton fisico genero el flanco de bajada
	uint32_t edge_capture = REG_READ(BUTTONS_PIO_BASE, PIO_EDGECAP_OFFSET);

	if (edge_capture != 0) {
		// 2. Si hay un boton capturado, procesarlo
		if ((bandera_boton_presionado == 0) && ((tiempo_segundos - tiempo_ultimo_click) >= 1 || tiempo_segundos == 0)){
			boton_detectado = edge_capture;
			bandera_boton_presionado = 1;
			tiempo_ultimo_click = tiempo_segundos;
		}

		// 3. Limpiar el registro de captura escribiendo un '1' bit activo
		REG_WRITE(BUTTONS_PIO_BASE, PIO_EDGECAP_OFFSET, edge_capture);
	}
}

void timer_isr(void) {
	// Limpiar la bandera de timeout del timer escribiendo un 0 en el registro de estado
	REG_WRITE(TIMER_BASE, TIMER_STATUS_OFFSET, 0);

	// El reloj de reproduccion solo avanza si estamos en estado PLAY
	if (estado_actual == STATE_PLAY) {
		tiempo_segundos++;

		// Conversion matematica a formato MM:SS
		segundos = tiempo_segundos % 60;
		minutos = tiempo_segundos / 60;

		// Levantamos la bandera para indicarle al Super Loop que debe actualizar la interfaz visual
		bandera_timer_un_segundo = 1;
	}
}

// Atributo especial requerido por el compilador GCC de Nios II para manejar el hardware
void handle_exception(void) __attribute__((section(".exceptions")));

void handle_exception(void) {
	// Leer que linea de IRQ de hardware esta solicitando atencion
	uint32_t ipending = NIOS2_READ_CTL_REG(NIOS2_CTL_IPENDING);

	// Verificar si la IRQ activa corresponde a los botones
	if (ipending & (1 << IRQ_BUTTONS)) {
		boton_isr(); // Llamamos a nuestra rutina especifica
	}

	if (ipending & (1 << IRQ_TIMER)) {
		timer_isr();
	}
}

void inicializar_interrupciones_botones(void) {
	// 1. Limpiar cualquier captura de flanco residual previa en el periferico
	REG_WRITE(BUTTONS_PIO_BASE, PIO_EDGECAP_OFFSET, 0xF);

	// 2. Continuar la mascara del periferico PIO para escuchar los 4 botones (KEY0 a KEY3)
	REG_WRITE(BUTTONS_PIO_BASE, PIO_IRQMASK_OFFSET, 0xF);

	// 3. Configurar la CPU Nios II: Activar la linea IRQ 3 en el registro ineable
	uint32_t ienable_actual = NIOS2_READ_CTL_REG(NIOS2_CTL_IENABLE);
	ienable_actual |= (1 << IRQ_BUTTONS);
	NIOS2_WRITE_CTL_REG(NIOS2_CTL_IENABLE, ienable_actual);

	// 4. Habilitar Interrupciones Globales en la CPU
	// (Poner a '1' el bit PIE en el registro status)
	uint32_t status_actual = NIOS2_READ_CTL_REG(NIOS2_CTL_STATUS);
	status_actual |= NIOS2_STATUS_PIE_MASK;
	NIOS2_WRITE_CTL_REG(NIOS2_CTL_STATUS, status_actual);
}

void inicializar_timer_1s(void) {
	// Suponga un reloj de 50MHz (50,000,00 ciclos = 0x02FAF080)
	// En caso de usar otra frecuencia, solo se cambian estos valores correspondientes a los 32-bits del periodo
	uint32_t periodo = 50000000;
	uint16_t periodo_low = (uint16_t)(periodo & 0xFFFF);
	uint16_t periodo_high = (uint16_t)((periodo >> 16) & 0xFFFF);

	// 1. Detener el timer antes de configurarlo
	REG_WRITE(TIMER_BASE, TIMER_CONTROL_OFFSET, TIMER_CTRL_STOP);

	// 2. Escribir el periodo de conteo en los registros de hardware
	REG_WRITE(TIMER_BASE, TIMER_PERIODL_OFFSET, periodo_low);
	REG_WRITE(TIMER_BASE, TIMER_PERIODH_OFFSET, periodo_high);

	// 3. Configurar el Timer: ITO, CONT, START
	REG_WRITE(TIMER_BASE, TIMER_CONTROL_OFFSET, TIMER_CTRL_ITO | TIMER_CTRL_CONT | TIMER_CTRL_START);

	// 4.Habilitar la linea IRQ_TIMER (bit 2) en el registro ienable de la CPU
	uint32_t ienable_actual = NIOS2_READ_CTL_REG(NIOS2_CTL_IENABLE);
	ienable_actual |= (1 << IRQ_TIMER);
	NIOS2_WRITE_CTL_REG(NIOS2_CTL_IENABLE, ienable_actual);
}

void actualizar_interfaz_visual(void) {
	// Asegurar que el display este habilitado y configurado en MODO TIEMPO (MM:SS)
	// Escribe en el registro de CONTROL del modulo personalizado
	REG_WRITE(HEX_DISPLAY_BASE, HEX_CONTROL_OFFSET, HEX_CTRL_ENABLE | HEX_CTRL_MODE_TIME);

	// Formatear los datos para el registro de DATA
	REG_WRITE(HEX_DISPLAY_BASE, HEX_DATA_OFFSET, tiempo_segundos);

	// Reflejar el estado de las pistas en los LEDs fisicos de la placa como apoyo visual
	REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, cancion_actual);
}

int main(void) {
	// Inicializar todo el hardware
	inicializar_interrupciones_botones();
	inicializar_timer_1s();

	// Encender un LED indicador en la tarjeta
	REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, 0x01);

	// SUPER LOOP - MOTOR DE CONTROL DEL REPRODUCTOR
	while(1) {
		// 1. GESTION DE BOTONES (Rutinas de control por interrupcion)
		if (bandera_boton_presionado) {
			// Consumir bandera de evento
			bandera_boton_presionado = 0;

			// BOTON 0 (KEY0): Comutador de Play/Pausa
			if (boton_detectado & (1 << 0)) {
				if (estado_actual == STATE_PLAY) {
					estado_actual = STATE_PAUSE;
				} else {
					estado_actual = STATE_PLAY;
				}
				actualizar_interfaz_visual();
			}

			// BOTON 1 (KEY1): Detener Reproduccion
			if (boton_detectado & (1 << 1)) {
				estado_actual = STATE_STOP;
				tiempo_segundos = 0;
				minutos = 0;
				segundos = 0;
				actualizar_interfaz_visual();
			}

			// BOTON 2 (KEY2): Siguiente Cancion
			if (boton_detectado & (1 << 2)) {
				// Paso de seguridad, cambiar momentaneamente a STOP
				estado_actual = STATE_STOP;
				actualizar_interfaz_visual();

				tiempo_segundos = 0;
				minutos = 0;
				segundos = 0;
				cancion_actual++;
				if (cancion_actual > TOTAL_CANCIONES) {
					// Volver a la primera pista
					cancion_actual = 1;
				}

				// Volver a paner en PLAY en la nueva pista
				estado_actual = STATE_PLAY;
				actualizar_interfaz_visual();
			}

			// BOTON3 (KEY3): Cancion Anterior
			if (boton_detectado & (1 << 3)) {
				// Paso de seguridad, cambiar momentaneamente a STOP
				estado_actual = STATE_STOP;
				actualizar_interfaz_visual();

				tiempo_segundos = 0;
				minutos = 0;
				segundos = 0;
				cancion_actual--;
				if (cancion_actual < 1) {
					// Ir a la ultima pista
					cancion_actual = TOTAL_CANCIONES;
				}

				// Volver a paner en PLAY en la nueva pista
				estado_actual = STATE_PLAY;
				actualizar_interfaz_visual();
			}

			boton_detectado = 0; // Restablecer el registro de captura procesado
		}

		// 2. GESTION DEL TIMER (Control Temporal)
		if (bandera_timer_un_segundo) {
			bandera_timer_un_segundo = 0;

			// Actualizar los displays de 7-segmentos con el nuevo tiempo de reproduccion
			actualizar_interfaz_visual();

			// Monitoreo de finalizcion de pista para reproduccion continua
			// Se debe proporcionar una bandera o una variable que indique el final
			if (tiempo_segundos >= 180) {
				tiempo_segundos = 0;
				minutos = 0;
				segundos = 0;
				cancion_actual++;
				if (cancion_actual > TOTAL_CANCIONES) {
					cancion_actual = 1;
				}
				actualizar_interfaz_visual();
			}
		}
	}

	return 0;
}
