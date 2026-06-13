/*
 * main.c
 */

#include "memory_map.h"
#include "ipc_protocol.h"
#include "vga_ui.h"

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

// ===== Decodificador del protocolo IPC (FIFO HPS -> NIOS) =====
typedef enum { FIFO_IDLE, FIFO_META, FIFO_TEXT, FIFO_PCM } FifoState_t;
volatile FifoState_t fifo_state = FIFO_IDLE;

static uint32_t meta_buf[TRACK_META_WORDS];
static uint32_t meta_idx = 0;
static uint32_t text_buf[TRACK_TEXT_WORDS];
static uint32_t text_idx = 0;
static uint32_t pcm_left = 0;
// Solo se acepta un paquete de texto inmediatamente despues de un META valido.
// Evita que un falso match (datos PCM con byte alto 0xF0) escriba basura.
static uint32_t expect_text = 0;

// Metadatos de la pista actual (los llena el decodificador, los usa la VGA)
volatile uint32_t track_sample_rate  = 0;
volatile uint32_t track_channels     = 0;
volatile uint32_t track_bits         = 0;
volatile uint32_t track_total_bytes  = 0;
volatile uint32_t track_duration_sec = 0;
volatile uint32_t track_meta_valid   = 0;
char track_title[TRACK_TEXT_FIELD_BYTES];
char track_artist[TRACK_TEXT_FIELD_BYTES];
volatile uint32_t nueva_pista_ui = 0;   // 1 = llego pista nueva, refrescar VGA

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
	// MODO POLLING: el super-loop sondea el registro de captura de flanco del
	// PIO. NO se habilitan interrupciones de CPU (bit PIE): el handler de
	// excepciones de este proyecto es una funcion C sin guardado/restauracion
	// de contexto ni 'eret', y colgaba/reiniciaba el Nios al primer flanco de
	// boton (borraba la VGA). El sondeo cumple la misma funcion de forma segura.
	//
	// Solo limpiamos cualquier flanco residual; la captura de flanco la hace el
	// hardware del PIO de forma continua, independiente del bit PIE.
	REG_WRITE(BUTTONS_PIO_BASE, PIO_EDGECAP_OFFSET, 0xF);
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

// Decodifica el FIFO IPC: distingue comandos/metadatos de las muestras PCM.
// Solo las muestras PCM van a la IP de audio; los comandos y metadatos se
// interpretan (y los strings se guardan para la VGA). Se llama una vez por
// iteracion del super loop (procesa una palabra o una muestra por llamada).
void procesar_streaming_audio(void) {
	static uint32_t empty_streak = 0;
	uint32_t nivel = REG_READ(FIFO_OUT_CSR_BASE, FIFO_LEVEL_REG);

	// Resincronizacion: si el FIFO lleva mucho tiempo vacio, el HPS esta entre
	// streams (reseteado o pausa larga). Volver a IDLE para alinear con el
	// proximo CMD_TRACK_START. Durante el streaming normal el HPS mantiene el
	// FIFO lleno, asi que un vacio sostenido es un limite de trama seguro.
	if (nivel == 0) {
		if (fifo_state != FIFO_IDLE && ++empty_streak > 200000u) {
			fifo_state  = FIFO_IDLE;
			expect_text = 0;
			empty_streak = 0;
		}
		return;
	}
	empty_streak = 0;

	switch (fifo_state) {
		case FIFO_IDLE: {
			uint32_t word = REG_READ(FIFO_OUT_BASE, 0x00);
			switch (word & CMD_OPCODE_MASK) {
				case CMD_TRACK_START:
					// El HPS manda el numero de pista en el payload; reiniciar tiempo.
					cancion_actual  = CMD_PAYLOAD(word);
					tiempo_segundos = 0; minutos = 0; segundos = 0;
					meta_idx = 0; fifo_state = FIFO_META;
					break;
				case CMD_TRACK_TEXT:
					/* Aceptar el texto SOLO si viene justo despues de un META
					 * valido; asi un falso match desde datos PCM (cuando el
					 * stream se desincroniza) no escribe basura en la VGA. */
					if (expect_text) { text_idx = 0; fifo_state = FIFO_TEXT; }
					break;
				case CMD_BLOCK_READY: pcm_left = PCM_BLOCK_WORDS; fifo_state = FIFO_PCM; break;
				case CMD_TRACK_END:   /* fin de pista */ break;
				case CMD_PLAY:
					/* El HPS indica que la pista debe reproducirse: arrancar.
					 * KEY0 sigue sirviendo para pausar/reanudar. */
					estado_actual = STATE_PLAY;
					break;
				case CMD_PAUSE:
					estado_actual = STATE_PAUSE;
					break;
				default:              break;  /* palabra desconocida: descartar */
			}
			break;
		}

		case FIFO_META: {
			if (nivel == 0) return;
			meta_buf[meta_idx++] = REG_READ(FIFO_OUT_BASE, 0x00);
			if (meta_idx >= TRACK_META_WORDS) {
				if (meta_buf[0] == TRACK_META_MAGIC) {
					track_sample_rate = meta_buf[1];
					track_channels    = (meta_buf[2] >> 16) & 0xFFFF;
					track_bits        =  meta_buf[2] & 0xFFFF;
					track_total_bytes =  meta_buf[3];
					uint32_t br = track_sample_rate * track_channels * (track_bits / 8);
					track_duration_sec = br ? (track_total_bytes / br) : 0;
					track_meta_valid = 1;
					expect_text = 1;   // el texto valido viene justo despues
				}
				fifo_state = FIFO_IDLE;
			}
			break;
		}

		case FIFO_TEXT: {
			if (nivel == 0) return;
			text_buf[text_idx++] = REG_READ(FIFO_OUT_BASE, 0x00);
			if (text_idx >= TRACK_TEXT_WORDS) {
				int i;
				for (i = 0; i < TRACK_TEXT_FIELD_WORDS; i++) {
					uint32_t w = text_buf[i];
					track_title[i*4+0] = (char)( w        & 0xFF);
					track_title[i*4+1] = (char)((w >> 8)  & 0xFF);
					track_title[i*4+2] = (char)((w >> 16) & 0xFF);
					track_title[i*4+3] = (char)((w >> 24) & 0xFF);
					uint32_t a = text_buf[TRACK_TEXT_FIELD_WORDS + i];
					track_artist[i*4+0] = (char)( a        & 0xFF);
					track_artist[i*4+1] = (char)((a >> 8)  & 0xFF);
					track_artist[i*4+2] = (char)((a >> 16) & 0xFF);
					track_artist[i*4+3] = (char)((a >> 24) & 0xFF);
				}
				track_title[TRACK_TEXT_FIELD_BYTES - 1]  = '\0';
				track_artist[TRACK_TEXT_FIELD_BYTES - 1] = '\0';
				nueva_pista_ui = 1;   // metadatos completos: avisar al super loop
				expect_text = 0;      // ya consumido: no aceptar mas texto
				fifo_state = FIFO_IDLE;
			}
			break;
		}

		case FIFO_PCM: {
			if (pcm_left == 0) { fifo_state = FIFO_IDLE; return; }
			if (estado_actual != STATE_PLAY) return;   // pausa: no drenar el bloque
			if (nivel == 0) return;                     // aun no llegaron las muestras
			uint32_t dsp = REG_READ(AUDIO_SAMPLE_INPUT_BASE, SAMPLE_STATUS_OFFSET);
			if (dsp & SAMPLE_STATUS_FIFO_FULL) return;  // sin espacio en el DSP: esperar
			uint32_t word = REG_READ(FIFO_OUT_BASE, 0x00);
			REG_WRITE(AUDIO_SAMPLE_INPUT_BASE, SAMPLE_WRITE_OFFSET, word & 0xFFFF);
			pcm_left--;
			if (pcm_left == 0) fifo_state = FIFO_IDLE;
			break;
		}
	}
}

void actualizar_interfaz_visual(void) {
	// Asegurar que el display este habilitado y configurado en MODO TIEMPO (MM:SS)
	// Escribe en el registro de CONTROL del modulo personalizado
	REG_WRITE(HEX_DISPLAY_BASE, HEX_CONTROL_OFFSET, HEX_CTRL_ENABLE | HEX_CTRL_MODE_TIME);

	// Formatear los datos para el registro de DATA
	REG_WRITE(HEX_DISPLAY_BASE, HEX_DATA_OFFSET, tiempo_segundos);

	// Reflejar el estado de las pistas en los LEDs fisicos de la placa como apoyo visual
	// DEBUG: silenciado para que el HPS use los LEDs como indicador de etapa.
	// REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, cancion_actual);
}

int main(void) {
	// Inicializar todo el hardware
	inicializar_interrupciones_botones();
	inicializar_timer_1s();

	// Encender un LED indicador en la tarjeta
	// DEBUG: silenciado para que el HPS use los LEDs como indicador de etapa.
	// REG_WRITE(LEDS_PIO_BASE, PIO_DATA_OFFSET, 0x01);

	// Pantalla VGA: dibujar marco y etiquetas fijas (REQ-09)
	vga_ui_init();

	// Habilitar el FIFO de muestras del DSP (R_DSP) y poner el filtro en bypass.
	// Sin el enable, las muestras PCM escritas no fluyen al serializador/codec;
	// sin seleccionar filtro, filter_sel queda indefinido. Bypass = audio directo.
	REG_WRITE(AUDIO_SAMPLE_INPUT_BASE, SAMPLE_CONTROL_OFFSET, SAMPLE_CTRL_ENABLE);
	REG_WRITE(AUDIO_FILTER_CONTROL_BASE, FILTER_CONTROL_OFFSET, FILTER_SEL_BYPASS);

	// Vaciar datos viejos del FIFO IPC (p.ej. de una corrida anterior del Nios)
	// para que el decodificador arranque alineado con el stream del HPS.
	while (REG_READ(FIFO_OUT_CSR_BASE, FIFO_LEVEL_REG) > 0) {
		(void)REG_READ(FIFO_OUT_BASE, 0x00);
	}
	fifo_state = FIFO_IDLE;
	expect_text = 0;

	// SUPER LOOP - MOTOR DE CONTROL DEL REPRODUCTOR
	while(1) {
		// 0. SONDEO de botones: el PIO captura el flanco de bajada por hardware
		//    (registro edge-capture), lo leemos aqui sin interrupciones.
		uint32_t ec = REG_READ(BUTTONS_PIO_BASE, PIO_EDGECAP_OFFSET) & 0xF;
		if (ec) {
			REG_WRITE(BUTTONS_PIO_BASE, PIO_EDGECAP_OFFSET, ec);  // limpiar flancos
			boton_detectado = ec;
			bandera_boton_presionado = 1;
		}

		// 0b. SONDEO del timer: el timer pone su bandera de timeout cada 1 s.
		if (REG_READ(TIMER_BASE, TIMER_STATUS_OFFSET) & TIMER_STATUS_TO) {
			REG_WRITE(TIMER_BASE, TIMER_STATUS_OFFSET, 0);  // limpiar timeout
			if (estado_actual == STATE_PLAY) {
				tiempo_segundos++;
				segundos = tiempo_segundos % 60;
				minutos  = tiempo_segundos / 60;
				bandera_timer_un_segundo = 1;
			}
		}

		// 1. GESTION DE BOTONES (por sondeo)
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

			// BOTON 2 (KEY2) y BOTON 3 (KEY3): Siguiente / Anterior.
			// El HPS es el DUEÑO de la navegacion de pistas: lee KEY2/KEY3 por
			// el puente ligero y cambia la cancion que streamea. El NIOS NO debe
			// cambiarla por su cuenta (provocaria conflicto/desfase); solo refleja
			// la pista que el HPS le indica via CMD_TRACK_START. Por eso aqui se
			// ignoran KEY2/KEY3 (igual se limpio su flanco arriba).

			boton_detectado = 0; // Restablecer el registro de captura procesado
		}

		// 2. GESTION DEL TIMER (Control Temporal)
		if (bandera_timer_un_segundo) {
			bandera_timer_un_segundo = 0;

			// Actualizar los displays de 7-segmentos con el nuevo tiempo de reproduccion
			actualizar_interfaz_visual();

			// Refrescar el tiempo transcurrido en la VGA (MM:SS)
			vga_ui_set_elapsed(tiempo_segundos);

			// El avance de pista lo decide el HPS (al terminar la cancion o por
			// los botones Siguiente/Anterior). El NIOS solo refleja la pista que
			// el HPS le indica via CMD_TRACK_START, sin auto-avanzar.
		}

		// Refrescar la VGA cuando llega una pista nueva (titulo/artista/duracion)
		if (nueva_pista_ui) {
			nueva_pista_ui = 0;
			TrackInfo ti;
			int k;
			for (k = 0; k < UI_TEXT_MAX - 1 && track_title[k];  k++) ti.title[k]  = track_title[k];
			ti.title[k] = '\0';
			for (k = 0; k < UI_TEXT_MAX - 1 && track_artist[k]; k++) ti.artist[k] = track_artist[k];
			ti.artist[k] = '\0';
			ti.duration_sec = track_duration_sec;
			vga_ui_set_track(&ti);
		}

		procesar_streaming_audio();
	}

	return 0;
}

//  /* main.c  -  Prueba "hello world" del driver VGA (REQ-09)
//  *
//  * Objetivo: confirmar que la cadena NIOS -> char_buffer ->
//  * vga_controller -> monitor funciona. Escribe texto fijo en
//  * pantalla y demuestra las primitivas del driver.
//  *
//  * Si se ve el texto en el monitor VGA, la base de REQ-09 esta lista.
//  *
//  * R_SoC
//  */
   
// #include "vga.h"
// #include "sys/alt_stdio.h"

// int main(void) {
//     alt_putstr("Prueba VGA - REQ-09\n");

//     /* Limpiar la pantalla (el HW ya lo hace al reset, pero por las dudas) */
//     vga_clear();

//     /* Titulo centrado */
//     vga_print_centered(5, "REPRODUCTOR DE AUDIO - CE1113");
//     vga_print_centered(7, "Grupo 2 - TEC");

//     /* Texto de prueba en posiciones fijas */
//     vga_print(10, 12, "Hola mundo desde NIOS II!");
//     vga_print(10, 14, "Driver VGA funcionando.");

//     /* Demostrar el formato de tiempo (02:05) */
//     vga_print(10, 18, "Tiempo de ejemplo:");
//     vga_print_time(29, 18, 125);   /* 125 s = 02:05 */

//     /* Demostrar impresion de numeros */
//     vga_print(10, 20, "Numero de ejemplo:");
//     vga_print_uint(29, 20, 48000);

//     /* Marco simple en las esquinas para verificar los bordes 80x60 */
//     vga_putchar(0, 0, '+');
//     vga_putchar(VGA_COLS - 1, 0, '+');
//     vga_putchar(0, VGA_ROWS - 1, '+');
//     vga_putchar(VGA_COLS - 1, VGA_ROWS - 1, '+');

//     alt_putstr("Texto escrito en pantalla VGA.\n");

//     /* No hay nada mas que hacer: el char_buffer retiene el texto. */
//     while (1) {
//         /* bucle infinito; la pantalla mantiene lo escrito */
//     }

//     return 0;
// }
