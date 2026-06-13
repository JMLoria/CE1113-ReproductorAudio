/* =============================================================================
 * syscalls.c — Retargeting mínimo de newlib para bare-metal.
 * Redirige la salida estándar (printf -> _write) a la UART0 del HPS y provee
 * _sbrk para que malloc/printf tengan heap. El resto son stubs.
 * ========================================================================== */
#include <stdint.h>
#include <sys/stat.h>
#include "hps_uart.h"

int _write(int fd, const char* buf, int len) {
    (void)fd;
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\n') uart_putc('\r');
        uart_putc(buf[i]);
    }
    return len;
}

int _read(int fd, char* buf, int len) { (void)fd; (void)buf; (void)len; return 0; }
int _close(int fd)                    { (void)fd; return -1; }
int _lseek(int fd, int off, int dir)  { (void)fd; (void)off; (void)dir; return 0; }
int _isatty(int fd)                   { (void)fd; return 1; }
int _getpid(void)                     { return 1; }
int _kill(int pid, int sig)           { (void)pid; (void)sig; return -1; }
void _exit(int code)                  { (void)code; while (1) { } }

int _fstat(int fd, struct stat* st) {
    (void)fd;
    st->st_mode = S_IFCHR;            /* tratar la salida como un "char device" */
    return 0;
}

/* Heap definido por el linker script */
extern char __heap_start;
extern char __heap_end;

void* _sbrk(int incr) {
    static char* heap = &__heap_start;
    char* prev = heap;
    if (heap + incr > &__heap_end) {
        return (void*)-1;            /* sin memoria */
    }
    heap += incr;
    return prev;
}
