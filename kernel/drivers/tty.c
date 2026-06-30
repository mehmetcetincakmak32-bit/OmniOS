/* OmniOS — kernel/drivers/tty.c */
/* TTY/console device — /dev/console, /dev/tty0 */
/* SPDX-License-Identifier: MIT */

#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

#define TTY_BUF_SIZE  4096

typedef struct {
    int    initialized;
    char   input_buf[TTY_BUF_SIZE];
    size_t input_head;
    size_t input_tail;
    int    echo;
    int    line_buffer;
} tty_device_t;

static tty_device_t _console_tty;

void tty_init(void) {
    memset(&_console_tty, 0, sizeof(_console_tty));
    _console_tty.echo = 1;
    _console_tty.line_buffer = 1;
    _console_tty.initialized = 1;
    printf("[TTY] Console TTY initialized\n");
}

void tty_putchar(char c) {
    putchar(c);
}

void tty_puts(const char *s) {
    while (*s) tty_putchar(*s++);
}

int tty_readline(char *buf, size_t bufsize) {
    if (!_console_tty.initialized) return 0;

    size_t i = 0;
    while (i < bufsize - 1) {
        /* In real system: wait for interrupt from UART/input */
        /* For now, return empty */
        break;
    }
    buf[i] = '\0';
    return (int)i;
}

void tty_input_put(char c) {
    if (!_console_tty.initialized) return;

    size_t next = (_console_tty.input_head + 1) % TTY_BUF_SIZE;
    if (next == _console_tty.input_tail) return;

    _console_tty.input_buf[_console_tty.input_head] = c;
    _console_tty.input_head = next;

    if (_console_tty.echo) tty_putchar(c);
}

int tty_input_get(char *c) {
    if (_console_tty.input_tail == _console_tty.input_head) return 0;
    *c = _console_tty.input_buf[_console_tty.input_tail];
    _console_tty.input_tail = (_console_tty.input_tail + 1) % TTY_BUF_SIZE;
    return 1;
}
