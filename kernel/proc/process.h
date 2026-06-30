/* OmniOS — kernel/proc/process.h */
/* Process and thread management — ARM64 + x86_64 */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_PROC_H
#define OMNIOS_PROC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../mm/vmm.h"

#define MAX_PROCESSES 1024
#define MAX_THREADS   4096
#define PROC_NAME_LEN 64
#define MAX_OPEN_FILES 128
#define MAX_PIPES     64

/* Process states */
typedef enum {
    PROC_CREATED    = 0,
    PROC_RUNNING    = 1,
    PROC_READY      = 2,
    PROC_BLOCKED    = 3,
    PROC_SLEEPING   = 4,
    PROC_ZOMBIE     = 5,
    PROC_TERMINATED = 6,
} process_state_t;

/* ── Architecture-specific context ───────────────────────────────── */

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) context_x86_64_t;

#ifdef __aarch64__
typedef struct {
    uint64_t x[31];
    uint64_t sp_el0;
    uint64_t elr_el1;
    uint64_t spsr_el1;
} __attribute__((packed)) context_arm64_t;
typedef context_arm64_t context_t;
#else
typedef context_x86_64_t context_t;
#endif

/* ── Open file descriptor ──────────────────────────────────────────── */

typedef struct {
    int    fd;
    int    type;   /* 0=none, 1=pipe, 2=tty, 3=dev */
    int    pipe_id;
    void  *private_data;
    int    flags;
    bool   in_use;
} file_descriptor_t;

/* ── Signal info ──────────────────────────────────────────────────── */

#define SIG_MAX       32
#define SIG_KILL      9
#define SIG_TERM      15
#define SIG_CHLD      17

typedef struct {
    uint64_t pending;     /* Bitmask of pending signals */
    uint64_t blocked;     /* Bitmask of blocked signals */
    void    *handlers[SIG_MAX];  /* Signal handlers */
    int      sig_ delivering;
} signal_info_t;

/* ── Thread control block ────────────────────────────────────────── */

typedef struct thread {
    uint32_t       tid;
    uint32_t       pid;
    context_t      context;
    uintptr_t      kernel_stack;
    uintptr_t      kernel_stack_top;
    uintptr_t      user_stack;
    uintptr_t      user_stack_top;
    process_state_t state;
    int            priority;
    int            cpu_affinity;
    uint64_t       wakeup_time;
    uint64_t       total_ticks;
    void          *sched_next;
    signal_info_t  signals;
    bool           used;
} thread_t;

/* ── Process control block ───────────────────────────────────────── */

typedef struct process {
    uint32_t       pid;
    uint32_t       ppid;
    char           name[PROC_NAME_LEN];
    process_state_t state;
    vm_space_t    *vm_space;
    uint32_t       thread_count;
    thread_t       threads[MAX_THREADS];
    uint32_t       first_thread;
    uint64_t       start_time;
    uint64_t       cpu_time;
    uint32_t       uid;
    uint32_t       gid;
    int            exit_code;
    file_descriptor_t fds[MAX_OPEN_FILES];
    signal_info_t  signals;
    bool           used;
} process_t;

/* ── Public API ──────────────────────────────────────────────────── */

void proc_init(void);
int  proc_create(const char *name, void (*entry)(void *), void *arg, int priority);
void proc_exit(int code);
int  proc_wait(int pid, int *status);
process_t *proc_get(int pid);
thread_t  *thread_get(int tid);
process_t *proc_get_current(void);
int  proc_fork(void);
int  proc_execve(const char *path, char *const argv[], char *const envp[]);

/* Context switch (assembly bridge) */
void context_switch(context_t *old, context_t *new);
void context_switch_to(context_t *new);

/* Scheduling interface */
thread_t *sched_pick_next(void);
void sched_add_thread(thread_t *thread);
void sched_remove_thread(thread_t *thread);

/* Userspace support */
int  proc_setup_userspace(process_t *proc, uintptr_t entry, uintptr_t stack, uintptr_t ttbr0);
void proc_enter_userspace(uintptr_t entry, uintptr_t stack, uintptr_t ttbr0);
void proc_set_current_thread(thread_t *t);

#endif /* OMNIOS_PROC_H */
