/* OmniOS — kernel/drivers/tty.h */
/* TTY/console interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_TTY_H
#define OMNIOS_TTY_H

#include <stddef.h>

void tty_init(void);
void tty_putchar(char c);
void tty_puts(const char *s);
int  tty_readline(char *buf, size_t bufsize);
void tty_input_put(char c);
int  tty_input_get(char *c);

#endif
