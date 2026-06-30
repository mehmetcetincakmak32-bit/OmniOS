/* OmniOS — kernel/proc/process.c */
/* Process and thread management — x86_64 + ARM64 */
/* SPDX-License-Identifier: MIT */

#include "process.h"
#include "signal.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

#ifdef __aarch64__
#include "../arch/arm64/exceptions.h"
#endif

static process_t processes[MAX_PROCESSES];
static uint32_t next_pid = 1;
static uint32_t next_tid = 1;
static thread_t *_current_thread = NULL;

/* ── Context switch: x86_64 ──────────────────────────────────────── */

#ifndef __aarch64__
__attribute__((naked)) void context_switch(context_t *old, context_t *new) {
    __asm__ volatile(
        "pushfq\n pushq %%r15\n pushq %%r14\n pushq %%r13\n pushq %%r12\n"
        "pushq %%r11\n pushq %%r10\n pushq %%r9\n  pushq %%r8\n"
        "pushq %%rbp\n pushq %%rdi\n pushq %%rsi\n pushq %%rdx\n"
        "pushq %%rcx\n pushq %%rbx\n pushq %%rax\n"
        "movq %%rsp, 0(%0)\n"
        "movq 0(%1), %%rsp\n"
        "popq %%rax\n popq %%rbx\n popq %%rcx\n popq %%rdx\n"
        "popq %%rsi\n popq %%rdi\n popq %%rbp\n"
        "popq %%r8\n  popq %%r9\n  popq %%r10\n popq %%r11\n"
        "popq %%r12\n popq %%r13\n popq %%r14\n popq %%r15\n"
        "popfq\n ret\n"
        : : "r"(old), "r"(new) : "memory");
}

__attribute__((naked)) void context_switch_to(context_t *new) {
    __asm__ volatile(
        "movq 0(%0), %%rsp\n"
        "popq %%rax\n popq %%rbx\n popq %%rcx\n popq %%rdx\n"
        "popq %%rsi\n popq %%rdi\n popq %%rbp\n"
        "popq %%r8\n  popq %%r9\n  popq %%r10\n popq %%r11\n"
        "popq %%r12\n popq %%r13\n popq %%r14\n popq %%r15\n"
        "popfq\n ret\n"
        : : "r"(new) : "memory");
}
#endif

/* ── Process creation ────────────────────────────────────────────── */

int proc_create(const char *name, void (*entry)(void *), void *arg, int priority) {
    int pid = -1;
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i].used) { pid = i; break; }
    }
    if (pid < 0) return -1;

    process_t *proc = &processes[pid];
    memset(proc, 0, sizeof(*proc));
    proc->pid = next_pid++;
    proc->state = PROC_READY;
    strncpy(proc->name, name ? name : "unnamed", PROC_NAME_LEN - 1);
    proc->vm_space = vmm_create_space();

    thread_t *thread = &proc->threads[0];
    memset(thread, 0, sizeof(*thread));
    thread->tid = next_tid++;
    thread->pid = proc->pid;
    thread->state = PROC_READY;
    thread->priority = priority;

    void *stack = (void *)pmm_alloc_pages(4);
    if (!stack) return -1;
    thread->kernel_stack_top = (uintptr_t)stack;
    thread->kernel_stack = thread->kernel_stack_top + 4 * PAGE_SIZE;

    memset(&thread->context, 0, sizeof(thread->context));

#ifdef __aarch64__
    thread->context.elr_el1 = (uint64_t)entry;
    thread->context.spsr_el1 = 0x3C5;
    thread->context.sp_el0 = 0;
    thread->context.x[0] = (uint64_t)arg;
#else
    thread->context.rip = (uint64_t)entry;
    thread->context.cs  = 0x08;
    thread->context.ss  = 0x10;
    thread->context.rsp = thread->kernel_stack;
    thread->context.rflags = 0x202;
    thread->context.rdi = (uint64_t)arg;
#endif

    thread->used = true;
    proc->thread_count = 1;
    proc->first_thread = 0;
    proc->used = true;
    proc->start_time = kernel_get_uptime_ms();
    signal_init_process(proc);

    sched_add_thread(thread);
    return proc->pid;
}

/* ── Process exit ────────────────────────────────────────────────── */

void proc_exit(int code) {
    process_t *proc = proc_get_current();
    if (!proc) {
#ifdef __aarch64__
        while (1) __asm__("wfi");
#else
        while (1) __asm__("sti; hlt");
#endif
    }
    proc->state = PROC_TERMINATED;
    proc->exit_code = code;
    printf("[PROC] pid=%d exit(%d)\n", proc->pid, code);

    /* Wake parent if waiting */
    signal_send(proc->ppid, SIG_CHLD);

    while (1) {
#ifdef __aarch64__
        __asm__("wfi");
#else
        __asm__("sti; hlt");
#endif
    }
}

int proc_wait(int pid, int *status) {
    (void)status;
    process_t *proc = proc_get(pid);
    if (!proc) return -1;
    while (proc->state != PROC_TERMINATED) {
#ifdef __aarch64__
        __asm__("wfi");
#else
        __asm__("pause");
#endif
    }
    return 0;
}

/* ── Process lookup ──────────────────────────────────────────────── */

process_t *proc_get(int pid) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].used && (int)processes[i].pid == pid)
            return &processes[i];
    }
    return NULL;
}

process_t *proc_get_current(void) {
    if (!_current_thread) return NULL;
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].used && processes[i].pid == _current_thread->pid)
            return &processes[i];
    }
    return NULL;
}

void proc_set_current_thread(thread_t *t) {
    _current_thread = t;
}

thread_t *thread_get(int tid) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i].used) continue;
        for (uint32_t j = 0; j < MAX_THREADS; j++) {
            if (processes[i].threads[j].used &&
                (int)processes[i].threads[j].tid == tid)
                return &processes[i].threads[j];
        }
    }
    return NULL;
}

/* ── Fork ─────────────────────────────────────────────────────────── */

int proc_fork(void) {
    process_t *parent = proc_get_current();
    if (!parent) return -1;

    int pid = -1;
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (!processes[i].used) { pid = i; break; }
    }
    if (pid < 0) return -1;

    process_t *child = &processes[pid];
    memcpy(child, parent, sizeof(process_t));
    child->pid = next_pid++;
    child->ppid = parent->pid;
    child->state = PROC_READY;
    child->used = true;
    child->thread_count = 1;
    child->start_time = kernel_get_uptime_ms();

    /* Allocate new VM space (simplified: same space) */
    child->vm_space = vmm_create_space();

    /* Copy threads */
    memset(child->threads, 0, sizeof(child->threads));
    memcpy(&child->threads[0], &parent->threads[0], sizeof(thread_t));
    child->threads[0].tid = next_tid++;
    child->threads[0].pid = child->pid;

    /* Return 0 in child, child PID in parent */
    child->threads[0].context.x[0] = 0;

    sched_add_thread(&child->threads[0]);
    printf("[FORK] child pid=%d from parent pid=%d\n", child->pid, parent->pid);
    return (int)child->pid;
}

/* ── Userspace setup ─────────────────────────────────────────────── */

int proc_setup_userspace(process_t *proc, uintptr_t entry, uintptr_t stack, uintptr_t ttbr0) {
    if (!proc) return -1;

    thread_t *t = &proc->threads[0];
    if (!t->used) return -1;

    t->user_stack = stack;
    t->user_stack_top = stack + 0x10000;

#ifdef __aarch64__
    t->context.elr_el1 = entry;
    t->context.spsr_el1 = 0x3C0;  /* EL0t, interrupts enabled */
    t->context.sp_el0  = stack;
    /* Set TTBR0 for EL0 via vm_space */
    if (proc->vm_space) proc->vm_space->pml4_phys = ttbr0;
#else
    t->context.rip = entry;
    t->context.cs  = 0x23;  /* User code segment */
    t->context.ss  = 0x1B;  /* User data segment */
    t->context.rsp = stack;
    t->context.rflags = 0x202;
#endif

    return 0;
}

void proc_enter_userspace(uintptr_t entry, uintptr_t stack, uintptr_t ttbr0) {
#ifdef __aarch64__
    arm64_enter_el0(entry, stack, ttbr0);
#else
    (void)entry; (void)stack; (void)ttbr0;
#endif
}

/* ── Init ──────────────────────────────────────────────────────────── */

void proc_init(void) {
    memset(processes, 0, sizeof(processes));
    _current_thread = NULL;
    printf("[PROC] Process manager: %d max\n", MAX_PROCESSES);
}
