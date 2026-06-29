/*
 * OmniOS System Call Interface
 * User-space to kernel-space transition layer
 */

#include "include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

static syscall_handler_t _syscall_table[SYS_MAX];

/* Forward declarations of handler implementations */
static status_t _sys_exit(uint32_t args[SYSCALL_MAX_ARGS]);
static status_t _sys_getpid(uint32_t args[SYSCALL_MAX_ARGS]);
static status_t _sys_sleep(uint32_t args[SYSCALL_MAX_ARGS]);
static status_t _sys_open(uint32_t args[SYSCALL_MAX_ARGS]);
static status_t _sys_close(uint32_t args[SYSCALL_MAX_ARGS]);
static status_t _sys_read(uint32_t args[SYSCALL_MAX_ARGS]);
static status_t _sys_write(uint32_t args[SYSCALL_MAX_ARGS]);
static status_t _sys_gettime(uint32_t args[SYSCALL_MAX_ARGS]);
static status_t _sys_sched_yield(uint32_t args[SYSCALL_MAX_ARGS]);

status_t syscall_init(void) {
    /* Clear table */
    memset(_syscall_table, 0, sizeof(_syscall_table));

    /* Register handlers */
    _syscall_table[SYS_EXIT]        = _sys_exit;
    _syscall_table[SYS_GETPID]      = _sys_getpid;
    _syscall_table[SYS_SLEEP]       = _sys_sleep;
    _syscall_table[SYS_OPEN]        = _sys_open;
    _syscall_table[SYS_CLOSE]       = _sys_close;
    _syscall_table[SYS_READ]        = _sys_read;
    _syscall_table[SYS_WRITE]       = _sys_write;
    _syscall_table[SYS_GETTIME]     = _sys_gettime;
    _syscall_table[SYS_SCHED_YIELD] = _sys_sched_yield;

    printf("[SYSCALL] %d sistem cagrisi kaydedildi\n", SYS_MAX);
    return STATUS_SUCCESS;
}

void syscall_handler(uint32_t syscall_num, uint32_t args[SYSCALL_MAX_ARGS], status_t* result) {
    if (!result) return;

    if (syscall_num >= SYS_MAX || !_syscall_table[syscall_num]) {
        *result = STATUS_NOT_IMPLEMENTED;
        return;
    }

    *result = _syscall_table[syscall_num](args);
}

/* ================================================================
 * System Call Implementations
 * ================================================================ */

static status_t _sys_exit(uint32_t args[SYSCALL_MAX_ARGS]) {
    int32_t code = (int32_t)args[0];
    printf("[SYSCALL] exit(%d) pid=%d\n", code, sched_get_current_pid());
    /* In real implementation: clean up process resources */
    return STATUS_SUCCESS;
}

static status_t _sys_getpid(uint32_t args[SYSCALL_MAX_ARGS]) {
    args[0] = (uint32_t)sched_get_current_pid();
    return STATUS_SUCCESS;
}

static status_t _sys_sleep(uint32_t args[SYSCALL_MAX_ARGS]) {
    time_t ms = (time_t)args[0];
    return sched_sleep(ms);
}

static status_t _sys_open(uint32_t args[SYSCALL_MAX_ARGS]) {
    const char* path = (const char*)(uintptr_t)args[0];
    uint32_t flags = args[1];
    if (!path) return STATUS_INVALID;

    int fd = vfs_open(path, flags);
    if (fd < 0) return (status_t)fd;
    args[0] = (uint32_t)fd;
    return STATUS_SUCCESS;
}

static status_t _sys_close(uint32_t args[SYSCALL_MAX_ARGS]) {
    int fd = (int)args[0];
    return vfs_close(fd);
}

static status_t _sys_read(uint32_t args[SYSCALL_MAX_ARGS]) {
    int fd = (int)args[0];
    uint8_t* buf = (uint8_t*)(uintptr_t)args[1];
    uint32_t size = args[2];
    uint32_t bytes_read = 0;
    status_t status = vfs_read(fd, buf, size, &bytes_read);
    args[0] = bytes_read;
    return status;
}

static status_t _sys_write(uint32_t args[SYSCALL_MAX_ARGS]) {
    int fd = (int)args[0];
    const uint8_t* buf = (const uint8_t*)(uintptr_t)args[1];
    uint32_t size = args[2];
    uint32_t bytes_written = 0;
    status_t status = vfs_write(fd, buf, size, &bytes_written);
    args[0] = bytes_written;
    return status;
}

static status_t _sys_gettime(uint32_t args[SYSCALL_MAX_ARGS]) {
    args[0] = (uint32_t)kernel_get_uptime_ms();
    return STATUS_SUCCESS;
}

static status_t _sys_sched_yield(uint32_t args[SYSCALL_MAX_ARGS]) {
    (void)args;
    return sched_yield();
}
