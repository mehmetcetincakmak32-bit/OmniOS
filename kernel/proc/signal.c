/* OmniOS — kernel/proc/signal.c */
/* Signal handling — delivery, blocking, handlers */
/* SPDX-License-Identifier: MIT */

#include "../include/omnios_kernel.h"
#include "process.h"
#include <stdio.h>
#include <string.h>

void signal_init_process(process_t *proc) {
    if (!proc) return;
    memset(&proc->signals, 0, sizeof(signal_info_t));
    proc->signals.pending = 0;
    proc->signals.blocked = 0;
}

void signal_send(int pid, int sig) {
    if (sig < 1 || sig >= SIG_MAX) return;
    process_t *proc = proc_get(pid);
    if (!proc) return;

    proc->signals.pending |= (1ULL << sig);
    printf("[SIGNAL] Sent SIG %d to PID %d\n", sig, pid);
}

int signal_check(process_t *proc) {
    if (!proc || proc->signals.pending == 0) return 0;

    for (int sig = 1; sig < SIG_MAX; sig++) {
        if (!(proc->signals.pending & (1ULL << sig))) continue;
        if (proc->signals.blocked & (1ULL << sig)) continue;

        proc->signals.pending &= ~(1ULL << sig);

        void *handler = proc->signals.handlers[sig];
        if ((uintptr_t)handler == 1) {
            /* SIG_IGN */
            continue;
        }
        if ((uintptr_t)handler == 0) {
            /* SIG_DFL */
            if (sig == SIG_KILL || sig == SIG_TERM) {
                printf("[SIGNAL] Process %d terminated by SIG %d\n", proc->pid, sig);
                proc_exit(128 + sig);
                return sig;
            }
            continue;
        }

        /* Custom handler would be called in userspace context */
        printf("[SIGNAL] Custom handler for SIG %d at %p\n", sig, handler);
        return sig;
    }
    return 0;
}

void signal_set_handler(process_t *proc, int sig, void *handler) {
    if (!proc || sig < 1 || sig >= SIG_MAX) return;
    proc->signals.handlers[sig] = handler;
}

void signal_block(process_t *proc, int sig) {
    if (!proc || sig < 1 || sig >= SIG_MAX) return;
    proc->signals.blocked |= (1ULL << sig);
}

void signal_unblock(process_t *proc, int sig) {
    if (!proc || sig < 1 || sig >= SIG_MAX) return;
    proc->signals.blocked &= ~(1ULL << sig);
}
