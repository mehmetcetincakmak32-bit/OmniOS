/*
 * OmniOS Standard I/O
 * Simple stdio implementation using syscalls
 */

#include "libc.h"
#include <stdarg.h>

/* Simple printf implementation */
static void _print_dec(uint32_t val, int* count) {
    char buf[16];
    int i = 0;
    if (val == 0) { buf[i++] = '0'; }
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i > 0) {
        sys_write(STDOUT_FILENO, &buf[--i], 1);
        if (count) (*count)++;
    }
}

static void _print_hex(uint32_t val, int* count) {
    const char* hex = "0123456789ABCDEF";
    char buf[8];
    sys_write(STDOUT_FILENO, "0x", 2);
    if (count) *count += 2;
    for (int i = 28; i >= 0; i -= 4) {
        char c = hex[(val >> i) & 0xF];
        sys_write(STDOUT_FILENO, &c, 1);
        if (count) (*count)++;
    }
}

static void _print_str(const char* s, int* count) {
    if (!s) s = "(null)";
    while (*s) {
        sys_write(STDOUT_FILENO, s, 1);
        s++;
        if (count) (*count)++;
    }
}

int printf(const char* fmt, ...) {
    int count = 0;
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd': {
                    int val = va_arg(args, int);
                    if (val < 0) {
                        sys_write(STDOUT_FILENO, "-", 1);
                        count++;
                        val = -val;
                    }
                    _print_dec((uint32_t)val, &count);
                    break;
                }
                case 'u':
                    _print_dec(va_arg(args, uint32_t), &count);
                    break;
                case 'x':
                case 'X':
                    _print_hex(va_arg(args, uint32_t), &count);
                    break;
                case 's':
                    _print_str(va_arg(args, const char*), &count);
                    break;
                case 'c': {
                    char c = (char)va_arg(args, int);
                    sys_write(STDOUT_FILENO, &c, 1);
                    count++;
                    break;
                }
                case '%':
                    sys_write(STDOUT_FILENO, "%", 1);
                    count++;
                    break;
                default:
                    sys_write(STDOUT_FILENO, fmt, 1);
                    count++;
                    break;
            }
        } else {
            sys_write(STDOUT_FILENO, fmt, 1);
            count++;
        }
        fmt++;
    }

    va_end(args);
    return count;
}
