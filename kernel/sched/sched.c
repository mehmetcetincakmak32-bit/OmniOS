/* OmniOS — kernel/sched/sched.c */
/* SMP scheduler implementation */
/* SPDX-License-Identifier: MIT */

#include "sched.h"
#include "../arch/x86_64/smp.h"
#include "../arch/x86_64/interrupts.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

#define MAX_CPU_RUNQUEUES MAX_CPUS
static runqueue_t runqueues[MAX_CPU_RUNQUEUES];
static thread_t *current_threads[MAX_CPUS];
static uint64_t tick_counter;

uint32_t sched_get_current_pid(void) {
    uint32_t cpu = smp_current_cpu();
    if (!current_threads[cpu]) return 0;
    return current_threads[cpu]->pid;
}

uint32_t sched_get_current_tid(void) {
    uint32_t cpu = smp_current_cpu();
    if (!current_threads[cpu]) return 0;
    return current_threads[cpu]->tid;
}

thread_t *sched_current_thread(void) {
    return current_threads[smp_current_cpu()];
}

static runqueue_t *get_runqueue(void) {
    uint32_t cpu = smp_current_cpu();
    if (cpu >= MAX_CPU_RUNQUEUES) cpu = 0;
    return &runqueues[cpu];
}

void sched_add_thread(thread_t *thread) {
    runqueue_t *rq = get_runqueue();
    if (rq->count >= MAX_RUNQUEUE_DEPTH) return;
    rq->queue[rq->tail] = thread;
    rq->tail = (rq->tail + 1) % MAX_RUNQUEUE_DEPTH;
    rq->count++;
    thread->state = PROC_READY;
}

void sched_remove_thread(thread_t *thread) {
    runqueue_t *rq = get_runqueue();
    for (uint32_t i = 0; i < rq->count; i++) {
        uint32_t idx = (rq->head + i) % MAX_RUNQUEUE_DEPTH;
        if (rq->queue[idx] == thread) {
            for (uint32_t j = i; j < rq->count - 1; j++) {
                uint32_t from = (rq->head + j + 1) % MAX_RUNQUEUE_DEPTH;
                uint32_t to = (rq->head + j) % MAX_RUNQUEUE_DEPTH;
                rq->queue[to] = rq->queue[from];
            }
            rq->tail = (rq->tail - 1 + MAX_RUNQUEUE_DEPTH) % MAX_RUNQUEUE_DEPTH;
            rq->count--;
            return;
        }
    }
}

thread_t *sched_pick_next(void) {
    runqueue_t *rq = get_runqueue();
    if (rq->count == 0) return NULL;

    /* Simple round-robin */
    thread_t *next = rq->queue[rq->head];
    rq->head = (rq->head + 1) % MAX_RUNQUEUE_DEPTH;
    rq->count--;

    if (next && next->state == PROC_READY) {
        next->state = PROC_RUNNING;
        return next;
    }
    return NULL;
}

void sched_tick(void) {
    tick_counter++;
    uint32_t cpu = smp_current_cpu();
    runqueue_t *rq = &runqueues[cpu];
    rq->total_ticks++;

    if (current_threads[cpu]) {
        current_threads[cpu]->total_ticks++;
    }
}

void sched_yield(void) {
    uint32_t cpu = smp_current_cpu();
    thread_t *current = current_threads[cpu];
    thread_t *next = sched_pick_next();

    if (!next) {
        if (current) current->state = PROC_RUNNING;
        return;
    }

    if (current && current->state == PROC_RUNNING) {
        current->state = PROC_READY;
        sched_add_thread(current);
    }

    current_threads[cpu] = next;
    context_switch(
        current ? &current->context : NULL,
        &next->context);
}

void sched_sleep(uint64_t ms) {
    thread_t *current = sched_current_thread();
    if (!current) return;
    current->wakeup_time = kernel_get_uptime_ms() + ms;
    current->state = PROC_SLEEPING;
    sched_yield();
}

void sched_wake(thread_t *thread) {
    if (!thread || thread->state != PROC_SLEEPING) return;
    thread->state = PROC_READY;
    sched_add_thread(thread);
}

void sched_balance(void) {
    /* Simple load balance: if one CPU has 2+ more threads than another,
       migrate one.  TODO: real pull/push for SMP. */
    uint32_t max_count = 0, min_count = 0xFFFFFFFF;
    int max_cpu = 0, min_cpu = 0;
    int ncpus = smp_online_cpus();

    for (int i = 0; i < ncpus; i++) {
        uint32_t c = runqueues[i].count;
        if (c > max_count) { max_count = c; max_cpu = i; }
        if (c < min_count) { min_count = c; min_cpu = i; }
    }
    if (max_count > min_count + 1 && max_count > 0 && min_cpu != max_cpu) {
        /* Migrate one from max to min (IPI-based) */
        (void)max_cpu;
        (void)min_cpu;
    }
}

void sched_init(void) {
    memset(runqueues, 0, sizeof(runqueues));
    memset(current_threads, 0, sizeof(current_threads));
    tick_counter = 0;

    int ncpus = smp_online_cpus();
    for (int i = 0; i < ncpus && i < MAX_CPU_RUNQUEUES; i++) {
        runqueues[i].cpu_index = i;
    }

    printf("[SCHED] SMP scheduler: %d CPUs, %d Hz\n",
        ncpus, 100);
}
