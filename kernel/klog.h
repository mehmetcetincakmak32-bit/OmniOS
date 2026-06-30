/* OmniOS — kernel/klog.h */
/* Kernel log interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_KLOG_H
#define OMNIOS_KLOG_H

#include <stddef.h>

void klog_init(void);
void klog_write(const char *msg);
void klog_printf(const char *fmt, ...);
int  klog_read(char *buf, size_t bufsize);
void klog_clear(void);
void __klog_putchar(char c);

#endif
