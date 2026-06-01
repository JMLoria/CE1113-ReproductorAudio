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

// Variables globales de estado compartidas (Volatiles para evitar optimizaciones del compilador)
volatile uint32_t bandera_boton_presionado = 0;
volatile uint32_t boton_detectado = 0;
volatile uint32_t bandera_timer_un_segundo = 0;

// Esta sera nuestra funcion ISR dedicada a los botones fisicos
void boton_isr(void) {
	// 1. Leer que boton fisico genero el flanco de bajada
	uint32_t edge_capture = REG_READ(BUTTONS_PIO_BASE, PIO_EDGECAP_OFFSET);

	// 2. Si hay un boton capturado, procesarlo
	if (edge_capture != 0) {
		boton_detectado = edge_capture;
		bandera_boton_presionado = 1;

		// 3. Limpiar el registro de captura escribiendo un '1' bit activo
		REG_WRITE(BUTTONS_PIO_BASE, PIO_EDGECAP_OFFSET, edge_capture);
	}
}

void timer_isr(void) {
	// Limpiar la bandera de timeout del timer escribiendo un 0 en el registro de estado
	REG_WRITE(TIMER_BASE, TIMER_STATUS_OFFSET, 0);
	bandera_timer_un_segundo = 1;
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

int main(void) {
	// Inicializar todo el hardware
	inicializar_interrupciones_botones();
	inicializar_timer_1s();

	// Encender un LED indicador en la tarjeta
	REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, 0x01);

	// SUPER LOOP (Consumo de banderas controladas por interrupcion)
	while(1) {
		// Consumo de interrupcion de botones
		if (bandera_boton_presionado) {
			bandera_boton_presionado = 0;
			if (boton_detectado & (1 << 0)) { // KEY0
				// Simula Play / Pause
				REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, 0x02);
			}
			if (boton_detectado & (1 << 1)) { // KEY1
				// Simula Stop
				REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, 0x04);
			}
			boton_detectado = 0;
		}

		//Consumo de interrupcion del Timer
		if (bandera_timer_un_segundo) {
			bandera_timer_un_segundo = 0;
			// Aqui se integrara el relog en la Semana 3
		}
	}

	return 0;
}
