/* OmniOS — kernel/proc/process.c */
/* Process and thread management */
/* SPDX-License-Identifier: MIT */

#include "process.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../arch/x86_64/paging.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

static process_t processes[MAX_PROCESSES];
static uint32_t next_pid = 1;
static uint32_t next_tid = 1;

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

int proc_create(const char *name, void (*entry)(void *), void *arg,
        int priority) {
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
    thread->kernel_stack_top = (uintptr_t)pmm_alloc_pages(4);
    if (!thread->kernel_stack_top) return -1;
    thread->kernel_stack = thread->kernel_stack_top + 4 * PAGE_SIZE;

    memset(&thread->context, 0, sizeof(thread->context));
    thread->context.rip = (uint64_t)entry;
    thread->context.cs  = 0x08;
    thread->context.ss  = 0x10;
    thread->context.rsp = thread->kernel_stack;
    thread->context.rflags = 0x202;
    thread->context.rdi = (uint64_t)arg;
    thread->used = true;
    proc->thread_count = 1;
    proc->first_thread = 0;
    proc->used = true;
    proc->start_time = kernel_get_uptime_ms();

    printf("[PROC] '%s' pid=%d tid=%d\n", proc->name, proc->pid, thread->tid);
    sched_add_thread(thread);
    return proc->pid;
}

void proc_exit(int code) {
    uint32_t pid = sched_get_current_pid();
    process_t *proc = proc_get(pid);
    if (!proc) return;
    proc->state = PROC_TERMINATED;
    printf("[PROC] pid=%d exit(%d)\n", pid, code);
    while (1) __asm__ volatile("sti; hlt");
}

int proc_wait(int pid, int *status) {
    (void)status;
    process_t *proc = proc_get(pid);
    if (!proc) return -1;
    while (proc->state != PROC_TERMINATED) __asm__("pause");
    return 0;
}

process_t *proc_get(int pid) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].used && (int)processes[i].pid == pid)
            return &processes[i];
    }
    return NULL;
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

void proc_init(void) {
    memset(processes, 0, sizeof(processes));
    printf("[PROC] Process manager: %d max\n", MAX_PROCESSES);
}
