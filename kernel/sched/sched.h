/* OmniOS — kernel/sched/sched.h */
/* SMP-aware scheduler — per-CPU runqueues, EEVDF-like */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_SCHED_H
#define OMNIOS_SCHED_H

#include <stdint.h>
#include "../proc/process.h"

#define MAX_PRIORITY 32
#define TIME_SLOT_MS 10
#define MAX_RUNQUEUE_DEPTH 256

/* Per-CPU runqueue */
typedef struct {
    thread_t *queue[MAX_RUNQUEUE_DEPTH];
    uint32_t  head;
    uint32_t  tail;
    uint32_t  count;
    uint64_t  total_ticks;
    uint32_t  cpu_index;
} runqueue_t;

void sched_init(void);
void sched_tick(void);
void sched_add_thread(thread_t *thread);
void sched_remove_thread(thread_t *thread);
thread_t *sched_pick_next(void);
void sched_yield(void);
void sched_sleep(uint64_t ms);
void sched_wake(thread_t *thread);

/* Current thread info */
uint32_t sched_get_current_pid(void);
uint32_t sched_get_current_tid(void);
thread_t *sched_current_thread(void);

/* Load balancing */
void sched_balance(void);

#endif /* OMNIOS_SCHED_H */
