/* OmniOS — kernel/arch/x86_64/smp.h */
/* SMP boot + per-CPU management */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_SMP_H
#define OMNIOS_SMP_H

#include <stdint.h>
#include <stdbool.h>
#include "cpu.h"

#define AP_TRAMPOLINE_ADDR 0x8000
#define MAX_CPUS 256

typedef void (*ap_entry_func)(uint32_t cpu_index);

void smp_init(void);
void smp_boot_aps(void);
int smp_online_cpus(void);
uint32_t smp_current_cpu(void);
void smp_send_ipi(uint32_t cpu, uint8_t vector);
void smp_send_broadcast(uint8_t vector);
void smp_percpu_init(uint32_t cpu_index);

#endif /* OMNIOS_SMP_H */
