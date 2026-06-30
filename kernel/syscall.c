/* OmniOS — kernel/syscall.c */
/* System calls — full implementation for userspace */
/* SPDX-License-Identifier: MIT */

#include "include/omnios_kernel.h"
#include "proc/process.h"
#include "proc/signal.h"
#include "sched/sched.h"
#include "fs/elf.h"
#include "fs/initramfs.h"
#include "ipc/pipe.h"
#include <stdio.h>
#include <string.h>

#ifdef __aarch64__
#include "arch/arm64/arm64.h"
#endif

#define SYSCALL_MAX      64

#define SYS_EXIT         1
#define SYS_WRITE        2
#define SYS_READ         3
#define SYS_OPEN         4
#define SYS_CLOSE        5
#define SYS_SLEEP        6
#define SYS_GETPID       7
#define SYS_FORK         8
#define SYS_EXECVE       9
#define SYS_BRK          10
#define SYS_SCHED_YIELD  11
#define SYS_GETTIME      12
#define SYS_POWEROFF     13
#define SYS_REBOOT       14

typedef long (*syscall_fn)(long arg0, long arg1, long arg2, long arg3, long arg4, long arg5);

static initramfs_t _sys_initramfs;
static int _initramfs_loaded = 0;

void syscall_set_initramfs(const void *data, size_t size) {
    initramfs_init(&_sys_initramfs, data, size);
    _initramfs_loaded = 1;
}

static long sys_exit(long code, long a1, long a2, long a3, long a4, long a5) {
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    printf("[SYSCALL] exit(%ld)\n", code);
    process_t *p = process_get_current();
    if (p) process_exit(p, (status_t)code);
    while (1) __asm__("wfi");
}

static long sys_write(long fd, long buf, long count, long a3, long a4, long a5) {
    (void)a3; (void)a4; (void)a5;
    if (fd == 1 || fd == 2) {
        const char *str = (const char *)buf;
        for (long i = 0; i < count; i++) putchar(str[i]);
    }
    return count;
}

static long sys_read(long fd, long buf, long count, long a3, long a4, long a5) {
    (void)fd; (void)buf; (void)count; (void)a3; (void)a4; (void)a5;
    return 0;
}

static long sys_open(long path, long flags, long mode, long a3, long a4, long a5) {
    (void)path; (void)flags; (void)mode; (void)a3; (void)a4; (void)a5;
    return -1;
}

static long sys_close(long fd, long a1, long a2, long a3, long a4, long a5) {
    (void)fd; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return 0;
}

static long sys_sleep(long ms, long a1, long a2, long a3, long a4, long a5) {
    (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    sched_sleep((time_t)ms);
    return 0;
}

static long sys_getpid(long a0, long a1, long a2, long a3, long a4, long a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    process_t *p = process_get_current();
    return p ? (long)p->pid : -1;
}

static long sys_fork(long a0, long a1, long a2, long a3, long a4, long a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    printf("[SYSCALL] fork()\n");
    return -1;
}

static long sys_execve(long path, long argv, long envp, long a3, long a4, long a5) {
    (void)argv; (void)envp; (void)a3; (void)a4; (void)a5;

    const char *fname = (const char *)path;
    if (!fname || !_initramfs_loaded) return -1;

    printf("[SYSCALL] execve(%s)\n", fname);

    initramfs_entry_t entry;
    if (initramfs_find(&_sys_initramfs, fname, &entry) <= 0) {
        printf("[SYSCALL] File not found in initramfs: %s\n", fname);
        return -1;
    }

    elf_loaded_t loaded;
    if (elf_load(entry.data, entry.size, &loaded) < 0) {
        printf("[SYSCALL] ELF load failed: %s\n", fname);
        return -1;
    }

    printf("[SYSCALL] Loading %s at entry 0x%llx\n", fname,
        (unsigned long long)loaded.entry);

#ifdef __aarch64__
    /* Create user page table and stack */
    uintptr_t user_pgd = arm64_create_user_pgd();
    uintptr_t user_stack_phys = pmm_alloc_pages(4);
    uintptr_t user_stack_virt = 0x7FFFF0000ULL;

    /* Map init binary at its load address */
    uintptr_t load_start = loaded.load_base & ~0xFFFULL;
    uintptr_t load_end   = ((loaded.load_base + loaded.size) + 0xFFF) & ~0xFFFULL;
    size_t load_pages = (load_end - load_start) / PAGE_SIZE;

    for (size_t i = 0; i < load_pages; i++) {
        arm64_map_user_page(user_pgd, load_start + i * PAGE_SIZE,
            load_start + i * PAGE_SIZE, 1, 1);
    }

    /* Map stack */
    for (int i = 0; i < 4; i++) {
        arm64_map_user_page(user_pgd,
            user_stack_virt + i * PAGE_SIZE,
            user_stack_phys + i * PAGE_SIZE, 1, 0);
    }

    /* Set up process for userspace */
    process_t *proc = proc_get_current();
    if (proc) {
        proc_setup_userspace(proc, loaded.entry, user_stack_virt + 0x10000 - 8, user_pgd);
    }

    /* Enter userspace */
    arm64_set_ttbr0(user_pgd);
    proc_enter_userspace(loaded.entry, user_stack_virt + 0x10000 - 8, user_pgd);
#else
    void (*entry_fn)(void) = (void (*)(void))(uintptr_t)loaded.entry;
    entry_fn();
#endif

    return 0;
}

static long sys_fork(long a0, long a1, long a2, long a3, long a4, long a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return (long)proc_fork();
}

static long sys_yield(long a0, long a1, long a2, long a3, long a4, long a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    sched_yield();
    return 0;
}

static long sys_gettime(long a0, long a1, long a2, long a3, long a4, long a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    return (long)kernel_get_uptime_ms();
}

static long sys_poweroff(long a0, long a1, long a2, long a3, long a4, long a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    kernel_poweroff();
    return 0;
}

static long sys_reboot(long a0, long a1, long a2, long a3, long a4, long a5) {
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4; (void)a5;
    kernel_reboot();
    return 0;
}

static syscall_fn _syscall_table[SYSCALL_MAX] = {
    [SYS_EXIT]        = sys_exit,
    [SYS_WRITE]       = sys_write,
    [SYS_READ]        = sys_read,
    [SYS_OPEN]        = sys_open,
    [SYS_CLOSE]       = sys_close,
    [SYS_SLEEP]       = sys_sleep,
    [SYS_GETPID]      = sys_getpid,
    [SYS_FORK]        = sys_fork,
    [SYS_EXECVE]      = sys_execve,
    [SYS_BRK]         = sys_brk,
    [SYS_SCHED_YIELD] = sys_yield,
    [SYS_GETTIME]     = sys_gettime,
    [SYS_POWEROFF]    = sys_poweroff,
    [SYS_REBOOT]      = sys_reboot,
};

status_t syscall_init(void) {
    printf("[SYSCALL] Syscall table ready (%d entries)\n", SYSCALL_MAX);
    return STATUS_SUCCESS;
}

long syscall_dispatch(uint32_t num, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5) {
    if (num >= SYSCALL_MAX || !_syscall_table[num]) {
        printf("[SYSCALL] Unknown syscall %u\n", num);
        return -1;
    }
    return _syscall_table[num](arg0, arg1, arg2, arg3, arg4, arg5);
}

/* ARM64 SVC handler — called from exception vector */
void arm64_svc_handler(uint32_t num, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long *result) {
    *result = syscall_dispatch(num, arg0, arg1, arg2, arg3, arg4, arg5);
}

/* ── Compatibility API (old syscalls.c interface) ───────────────── */

void syscall_handler(uint32_t syscall_num, uint32_t args[SYSCALL_MAX_ARGS], status_t *result) {
    if (!result) return;
    long r = syscall_dispatch(syscall_num,
        (long)args[0], (long)args[1], (long)args[2],
        (long)args[3], (long)args[4], (long)args[5]);
    *result = (r < 0) ? (status_t)r : STATUS_SUCCESS;
}
