/* OmniOS — kernel/klog.c */
/* Kernel log ring buffer */
/* SPDX-License-Identifier: MIT */

#include "include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

#define KLOG_SIZE       32768
#define KLOG_MAX_LINE   256

static char _klog_buffer[KLOG_SIZE];
static size_t _klog_head = 0;
static size_t _klog_tail = 0;
static int _klog_initialized = 0;

void klog_init(void) {
    memset(_klog_buffer, 0, KLOG_SIZE);
    _klog_head = 0;
    _klog_tail = 0;
    _klog_initialized = 1;
}

void klog_write(const char *msg) {
    if (!_klog_initialized) return;

    size_t len = strlen(msg);
    if (len == 0) return;

    for (size_t i = 0; i < len; i++) {
        _klog_buffer[_klog_head] = msg[i];
        _klog_head = (_klog_head + 1) % KLOG_SIZE;
        if (_klog_head == _klog_tail)
            _klog_tail = (_klog_tail + 1) % KLOG_SIZE;
    }
}

void klog_printf(const char *fmt, ...) {
    char buf[KLOG_MAX_LINE];
    /* Simplified: just print directly; real impl would use vsnprintf */
    (void)fmt;
    (void)buf;
}

int klog_read(char *buf, size_t bufsize) {
    if (!_klog_initialized || !buf || bufsize == 0) return 0;

    size_t written = 0;
    while (_klog_tail != _klog_head && written < bufsize - 1) {
        buf[written++] = _klog_buffer[_klog_tail];
        _klog_tail = (_klog_tail + 1) % KLOG_SIZE;
    }
    buf[written] = '\0';
    return (int)written;
}

void klog_clear(void) {
    memset(_klog_buffer, 0, KLOG_SIZE);
    _klog_head = 0;
    _klog_tail = 0;
}

/* Hook into printf */
void __klog_putchar(char c) {
    if (!_klog_initialized) return;
    _klog_buffer[_klog_head] = c;
    _klog_head = (_klog_head + 1) % KLOG_SIZE;
    if (_klog_head == _klog_tail)
        _klog_tail = (_klog_tail + 1) % KLOG_SIZE;
}
