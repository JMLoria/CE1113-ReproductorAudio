#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>


// Definicion de estados 
typedef enum {
    STATE_STOP,
    STATE_PLAY,
    STATE_PAUSE,
} PlayerState_t; 

// Variables globales compartidas (Volatiles, simulando registros/bandera de hardware)
volatile PlayerState_t estado_actual = STATE_STOP;
volatile int cancion_actual = 1;
const int TOTAL_CANCIONES = 10; // Soporte de al menos 10 canciones
volatile int tiempo_segundos = 0;
volatile bool cambiar_pista_auto = false;
volatile bool ejecutar_sistema = true;

// Prototipos de funciones logicas
void actualizar_displays(void);
void *hilo_interrupciones_teclado(void *arg);
void *hilo_timer_un_segundo(void *arg);

int main(void) {
    pthread_t thread_teclado, thread_timer;

    printf("===================================================\n");
    printf("   SIMULADOR DE CONTROL BARE-METAL - REPRODUCTOR    \n");
    printf("===================================================\n");
    printf("Controles simulados:\n");
    printf("  [p] - Play / Pausa (Simula KEY0)\n");
    printf("  [s] - Stop         (Simula KEY1)\n");
    printf("  [n] - Siguiente    (Simula KEY2)\n");
    printf("  [b] - Anterior     (Simula KEY3)\n");
    printf("  [q] - Salir de la prueba\n");
    printf("------------------------------------------------===\n");

    // Lanzar hilos que simulan las interrupciones del hardware
    pthread_create(&thread_teclado, NULL, hilo_interrupciones_teclado, NULL);
    pthread_create(&thread_timer, NULL, hilo_timer_un_segundo, NULL);

    // El SUPER LOOP del Nios II (Maquina de Estados) 
    while (ejecutar_sistema) {
        switch (estado_actual) {
            case STATE_STOP: // En STOP no incrementa tiempo ni despacha audio
                usleep(100000);
                break;
            
            case STATE_PLAY: // Simulacion de reproduccion
                if (tiempo_segundos >= 15) { // Simulamos que cada cancion dura 15 segundos para la prueba
                    cambiar_pista_auto = true;
                }

                if (cambiar_pista_auto) {
                    cambiar_pista_auto = false;
                    tiempo_segundos = 0;
                    cancion_actual++;
                    if (cancion_actual > TOTAL_CANCIONES) {
                        cancion_actual = 1;
                    }
                    printf("\n[NOTIFICACION HW]: Cancion finalizada. Reproduciendo siguiente pista de forma automatica...\n");
                    actualizar_displays();
                }
                usleep(100000);
                break;
            
            case STATE_PAUSE: // En PAUSA el flujo de audio y tiempo se congelan
                usleep(100000);
                break;
        }
    }

    // Esperar a que los hilos finalicen antes de cerrar
    pthread_join(thread_teclado, NULL);
    pthread_join(thread_timer, NULL);

    printf("\nPrueba finalizada con exito. Codigo de estados verificado.\n");
    return 0;
}

// Funcion que simula la escritura en el periferico de los Displays de 7-Segmentos 
void actualizar_displays(void) {
    int minutos = tiempo_segundos / 60;
    int segundos = tiempo_segundos % 60;

    const char* str_estado = (estado_actual == STATE_PLAY)  ? "PLAY" :
                             (estado_actual == STATE_PAUSE) ? "PAUSA" : "STOP";

    printf("\n[Displays 7-seg / UI] Estado: %s | Pista: %02d/%02d | Tiempo: %02d:%02d", 
            str_estado, cancion_actual, TOTAL_CANCIONES, minutos, segundos);
    fflush(stdout);
}

// HILO DE TECLADO: Simula las ISR de interrupcion de los botones fisicos
void *hilo_interrupciones_teclado(void *arg) {
    char tecla;
    while(ejecutar_sistema) {
        tecla = getchar(); 

        if (tecla == 'q' || tecla == 'Q') {
            ejecutar_sistema = false;
            break;
        }

        // Simulacion de Rutinas de Servicio de Interrupccion (ISR)
        switch(tecla) {
            case 'p': case 'P': // ISR Boton Play / Pausa
                if (estado_actual == STATE_PLAY){
                    estado_actual = STATE_PAUSE;
                } else {
                    estado_actual = STATE_PLAY;
                }
                actualizar_displays();
                break;

            case 's': case 'S': // ISR Boton Stop
                estado_actual = STATE_STOP;
                tiempo_segundos = 0;
                actualizar_displays();
                break;

            case 'n': case 'N': // ISR Boton Siguiente pista
                tiempo_segundos = 0;
                cancion_actual++;
                if (cancion_actual > TOTAL_CANCIONES) cancion_actual = 1;
                actualizar_displays();
                break;

            case 'b': case 'B': // ISR Boton Pista anterior
                tiempo_segundos = 0;
                cancion_actual--;
                if (cancion_actual < 1) cancion_actual = TOTAL_CANCIONES;
                actualizar_displays();
                break;

            default:
                break;
        }
    }
    return NULL;
}

// HILO_TIMER: Simula la interrupcion periodica por hardware de 1 segundo
void *hilo_timer_un_segundo(void *arg) {
    while (ejecutar_sistema) {
        sleep(1); 
        
        if (estado_actual == STATE_PLAY) {
            tiempo_segundos++;
            actualizar_displays();
        }
    }
    return NULL;
}