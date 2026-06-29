/*
 * OmniOS C Library
 * System call wrappers for userspace programs
 */

#ifndef OMNOS_LIBC_H
#define OMNOS_LIBC_H

#include "../../kernel/include/omnios_kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Standard file descriptors */
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

/* Open flags */
#define O_RDONLY        0x0000
#define O_WRONLY        0x0001
#define O_RDWR          0x0002
#define O_CREAT         0x0100
#define O_TRUNC         0x0200
#define O_APPEND        0x0400

/* ================================================================
 * System Call Wrappers
 * ================================================================ */

static inline status_t sys_exit(int code) {
    uint32_t args[SYSCALL_MAX_ARGS] = { (uint32_t)code };
    status_t result;
    syscall_handler(SYS_EXIT, args, &result);
    return result;
}

static inline pid_t sys_getpid(void) {
    uint32_t args[SYSCALL_MAX_ARGS] = {0};
    status_t result;
    syscall_handler(SYS_GETPID, args, &result);
    return (pid_t)args[0];
}

static inline status_t sys_sleep(time_t ms) {
    uint32_t args[SYSCALL_MAX_ARGS] = { (uint32_t)ms };
    status_t result;
    syscall_handler(SYS_SLEEP, args, &result);
    return result;
}

static inline int sys_open(const char* path, uint32_t flags) {
    uint32_t args[SYSCALL_MAX_ARGS] = { (uint32_t)(uintptr_t)path, flags };
    status_t result;
    syscall_handler(SYS_OPEN, args, &result);
    return result == STATUS_SUCCESS ? (int)args[0] : -1;
}

static inline status_t sys_close(int fd) {
    uint32_t args[SYSCALL_MAX_ARGS] = { (uint32_t)fd };
    status_t result;
    syscall_handler(SYS_CLOSE, args, &result);
    return result;
}

static inline int sys_read(int fd, void* buf, uint32_t size) {
    uint32_t args[SYSCALL_MAX_ARGS] = { (uint32_t)fd, (uint32_t)(uintptr_t)buf, size };
    status_t result;
    syscall_handler(SYS_READ, args, &result);
    return result == STATUS_SUCCESS ? (int)args[0] : -1;
}

static inline int sys_write(int fd, const void* buf, uint32_t size) {
    uint32_t args[SYSCALL_MAX_ARGS] = { (uint32_t)fd, (uint32_t)(uintptr_t)buf, size };
    status_t result;
    syscall_handler(SYS_WRITE, args, &result);
    return result == STATUS_SUCCESS ? (int)args[0] : -1;
}

static inline time_t sys_gettime(void) {
    uint32_t args[SYSCALL_MAX_ARGS] = {0};
    status_t result;
    syscall_handler(SYS_GETTIME, args, &result);
    return (time_t)args[0];
}

static inline status_t sys_yield(void) {
    uint32_t args[SYSCALL_MAX_ARGS] = {0};
    status_t result;
    syscall_handler(SYS_SCHED_YIELD, args, &result);
    return result;
}

#ifdef __cplusplus
}
#endif

#endif /* OMNOS_LIBC_H */
