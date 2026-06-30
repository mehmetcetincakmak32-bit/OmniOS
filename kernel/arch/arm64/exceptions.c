/* OmniOS — kernel/arch/arm64/exceptions.c */
/* ARM64 exception handler — dispatch SVC, IRQ, etc */
/* SPDX-License-Identifier: MIT */

#include "exceptions.h"
#include "../../syscall.h"
#include "../../include/omnios_kernel.h"
#include <stdio.h>

void arm64_exception_init(void) {
    /* Install vector table via VBAR_EL1 */
    extern uint8_t arm64_exception_vector[];
    uintptr_t vbar = (uintptr_t)arm64_exception_vector;
    __asm__ volatile("msr vbar_el1, %0" : : "r"(vbar));
    __asm__ volatile("isb");
    printf("[EXCEPT] VBAR_EL1 = 0x%lx\n", vbar);
}

void arm64_exception_handler(uint32_t type, uint32_t esr, exception_context_t *ctx) {
    switch (type) {
    case EXC_SVC_EL0: {
        /* System call from userspace */
        uint32_t syscall_num = esr & 0xFFFF;
        long result = syscall_dispatch(
            syscall_num,
            ctx->regs[0], ctx->regs[1], ctx->regs[2],
            ctx->regs[3], ctx->regs[4], ctx->regs[5]);
        ctx->regs[0] = (uint64_t)result;
        break;
    }
    case EXC_IRQ_EL1:
    case EXC_IRQ_EL0: {
        /* Handle physical IRQ */
        uint32_t irq_num = 0;
        __asm__ volatile("mrs %0, cntp_ctl_el0" : "=r"(irq_num));
        (void)irq_num;
        sched_tick();
        break;
    }
    case EXC_SYNC_EL1: {
        printf("[EXCEPT] Sync EL1: ESR=0x%x FAR=0x%lx\n", esr, ctx->far);
        break;
    }
    default:
        printf("[EXCEPT] Unhandled type=%d ESR=0x%x\n", type, esr);
        break;
    }
}
