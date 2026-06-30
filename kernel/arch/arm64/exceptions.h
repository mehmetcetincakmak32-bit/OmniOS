/* OmniOS — kernel/arch/arm64/exceptions.h */
/* ARM64 exception interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_ARM64_EXCEPTIONS_H
#define OMNIOS_ARM64_EXCEPTIONS_H

#include <stdint.h>

/* Saved context layout (must match exceptions.S) */
typedef struct {
    uint64_t regs[31];        /* x0-x30 */
    uint64_t spsr;
    uint64_t elr;
    uint64_t esr;
    uint64_t far;
} __attribute__((packed)) exception_context_t;

/* Exception types */
#define EXC_SYNC_EL1   0
#define EXC_IRQ_EL1    1
#define EXC_FIQ_EL1    2
#define EXC_SERR_EL1   3
#define EXC_SVC_EL0    4
#define EXC_IRQ_EL0    5
#define EXC_OTHER      6

void arm64_exception_handler(uint32_t type, uint32_t esr, exception_context_t *ctx);
void arm64_enter_el0(uint64_t entry, uint64_t sp, uint64_t ttbr0);
void arm64_exception_init(void);

#endif
