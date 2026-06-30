/* OmniOS — kernel/proc/signal.h */
/* Signal handling interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_SIGNAL_H
#define OMNIOS_SIGNAL_H

#define SIG_KILL  9
#define SIG_TERM  15
#define SIG_CHLD  17

struct process;

void signal_init_process(struct process *proc);
void signal_send(int pid, int sig);
int  signal_check(struct process *proc);
void signal_set_handler(struct process *proc, int sig, void *handler);
void signal_block(struct process *proc, int sig);
void signal_unblock(struct process *proc, int sig);

#endif
