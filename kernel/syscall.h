/* OmniOS — kernel/syscall.h */
/* System call interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_SYSCALL_H
#define OMNIOS_SYSCALL_H

#include <stdint.h>
#include <stddef.h>

status_t syscall_init(void);
long syscall_dispatch(uint32_t num, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5);
void syscall_set_initramfs(const void *data, size_t size);
void arm64_svc_handler(uint32_t num, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long *result);

#endif
