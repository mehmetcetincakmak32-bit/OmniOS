/*
 * OmniOS Scheduler
 * Multi-Level Feedback Queue (MLFQ) scheduler with priority support
 */

#include "include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

#define MAX_QUEUES          4
#define MAX_READY_QUEUE     256

typedef struct thread_control_block {
    tid_t       tid;
    pid_t       pid;
    char        name[32];
    thread_state_t state;
    uint32_t    priority;
    uint32_t    remaining_ticks;
    uint32_t    total_ticks;
    void*       stack_pointer;
    void*       entry_point;
    void*       arg;
    uint32_t    queue_level;
    /* Saved registers */
    uint64_t    regs[32];
    /* Kernel stack */
    uint8_t*    kernel_stack;
    uint32_t    kernel_stack_size;
    struct thread_control_block* next;
} tcb_t;

static tcb_t _thread_table[1024];
static uint32_t _thread_count = 0;
static tid_t _next_tid = 1;

static tcb_t* _ready_queues[MAX_QUEUES];
static tcb_t* _current_thread = NULL;
static pid_t _current_pid = 0;

static uint32_t _queue_quanta[MAX_QUEUES] = { 5, 10, 20, 40 };
static uint32_t _queue_priorities[MAX_QUEUES] = { 31, 24, 16, 8 };

/* Forward declarations */
static void _enqueue(tcb_t* thread);
static tcb_t* _dequeue(void);
static void _promote(tcb_t* thread);
static void _demote(tcb_t* thread);

status_t sched_init(void) {
    memset(_thread_table, 0, sizeof(_thread_table));
    for (int i = 0; i < MAX_QUEUES; i++) {
        _ready_queues[i] = NULL;
    }
    _current_thread = NULL;
    _current_pid = 0;
    _thread_count = 0;
    _next_tid = 1;
    printf("[SCHED] MLFQ zamanlayici baslatildi (%d kuyruk, %d ms quantum)\n",
           MAX_QUEUES, _queue_quanta[0]);
    return STATUS_SUCCESS;
}

tid_t sched_create_thread(pid_t pid, void (*entry)(void*), void* arg, uint32_t priority) {
    if (_thread_count >= 1024) return STATUS_NO_MEMORY;

    tcb_t* tcb = &_thread_table[_thread_count];
    memset(tcb, 0, sizeof(tcb_t));

    tcb->tid = _next_tid++;
    tcb->pid = pid;
    tcb->state = THREAD_READY;
    tcb->priority = priority;
    tcb->entry_point = (void*)entry;
    tcb->arg = arg;
    tcb->remaining_ticks = _queue_quanta[0];
    tcb->total_ticks = 0;
    tcb->queue_level = 0;
    snprintf(tcb->name, sizeof(tcb->name), "thread_%d", tcb->tid);

    /* Allocate kernel stack */
    tcb->kernel_stack_size = 8192;
    tcb->kernel_stack = (uint8_t*)kmalloc(tcb->kernel_stack_size);
    if (!tcb->kernel_stack) return STATUS_NO_MEMORY;

    /* Set up initial stack frame (architecture specific) */
    tcb->stack_pointer = (void*)((uintptr_t)tcb->kernel_stack + tcb->kernel_stack_size - 128);

    _thread_count++;

    /* Add to ready queue */
    _enqueue(tcb);

    return tcb->tid;
}

status_t sched_yield(void) {
    if (!_current_thread) return STATUS_ERROR;

    /* Save current thread state */
    if (_current_thread->state == THREAD_RUNNING) {
        _current_thread->state = THREAD_READY;
        _enqueue(_current_thread);
    }

    /* Pick next thread */
    tcb_t* next = _dequeue();
    if (next) {
        next->state = THREAD_RUNNING;
        next->remaining_ticks = _queue_quanta[next->queue_level];
        _current_thread = next;
        _current_pid = next->pid;

        /* Context switch - architecture specific */
        /* switch_context(&_current_thread->regs, next->regs); */
    }

    return STATUS_SUCCESS;
}

status_t sched_sleep(time_t ms) {
    if (!_current_thread) return STATUS_ERROR;
    _current_thread->state = THREAD_SLEEPING;
    sched_yield();
    return STATUS_SUCCESS;
}

status_t sched_wake(tid_t tid) {
    for (uint32_t i = 0; i < _thread_count; i++) {
        if (_thread_table[i].tid == tid) {
            if (_thread_table[i].state == THREAD_SLEEPING ||
                _thread_table[i].state == THREAD_BLOCKED) {
                _thread_table[i].state = THREAD_READY;
                _enqueue(&_thread_table[i]);
                return STATUS_SUCCESS;
            }
        }
    }
    return STATUS_NOT_FOUND;
}

void sched_tick(void) {
    if (!_current_thread) return;

    _current_thread->total_ticks++;
    _current_thread->remaining_ticks--;

    if (_current_thread->remaining_ticks == 0) {
        /* Time slice expired - demote if possible */
        _demote(_current_thread);
        sched_yield();
    }
}

pid_t sched_get_current_pid(void) {
    return _current_pid;
}

tid_t sched_get_current_tid(void) {
    return _current_thread ? _current_thread->tid : 0;
}

/* ================================================================
 * Internal Queue Management
 * ================================================================ */

static void _enqueue(tcb_t* thread) {
    uint32_t level = thread->queue_level;

    /* Priority boost: if thread has high priority, put in higher queue */
    if (thread->priority >= PRIORITY_MAX - 1) {
        level = 0;
    }

    if (level >= MAX_QUEUES) level = MAX_QUEUES - 1;
    thread->queue_level = level;

    /* Add to tail of queue */
    if (!_ready_queues[level]) {
        _ready_queues[level] = thread;
        thread->next = NULL;
    } else {
        tcb_t* tail = _ready_queues[level];
        while (tail->next) tail = tail->next;
        tail->next = thread;
        thread->next = NULL;
    }
}

static tcb_t* _dequeue(void) {
    /* Check queues from highest priority (0) to lowest (3) */
    for (int i = 0; i < MAX_QUEUES; i++) {
        if (_ready_queues[i]) {
            tcb_t* thread = _ready_queues[i];
            _ready_queues[i] = thread->next;
            thread->next = NULL;
            return thread;
        }
    }
    return NULL;
}

static void _promote(tcb_t* thread) {
    /* Priority boost: move to higher queue */
    if (thread->queue_level > 0) {
        thread->queue_level--;
        thread->remaining_ticks = _queue_quanta[thread->queue_level];
    }
}

static void _demote(tcb_t* thread) {
    /* Demote to lower queue if using full quantum */
    if (thread->queue_level < MAX_QUEUES - 1) {
        thread->queue_level++;
        thread->remaining_ticks = _queue_quanta[thread->queue_level];
    }
}

/* ================================================================
 * Scheduler Statistics
 * ================================================================ */

uint32_t sched_get_thread_count(void) {
    return _thread_count;
}

uint32_t sched_get_queue_count(uint32_t level) {
    uint32_t count = 0;
    tcb_t* t = _ready_queues[level];
    while (t) { count++; t = t->next; }
    return count;
}

void sched_print_info(void) {
    printf("[SCHED] Threads: %d, Current: tid=%d pid=%d\n",
           _thread_count,
           _current_thread ? _current_thread->tid : 0,
           _current_pid);
    for (int i = 0; i < MAX_QUEUES; i++) {
        printf("[SCHED]  Queue %d: %d threads ready\n",
               i, sched_get_queue_count(i));
    }
}
