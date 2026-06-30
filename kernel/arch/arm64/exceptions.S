/* OmniOS — kernel/arch/arm64/exceptions.S */
/* ARM64 exception vectors — EL1 handlers + EL0 transition */
/* SPDX-License-Identifier: MIT */

.section .text.exceptions, "ax"
.align 11  /* 2KB aligned vector table */

.global arm64_exception_vector
arm64_exception_vector:
    /* Current EL with SP0 (unused) */
    .align 7; b sync_el1t_sp0
    .align 7; b irq_el1t_sp0
    .align 7; b fiq_el1t_sp0
    .align 7; b serr_el1t_sp0

    /* Current EL with SPx */
    .align 7; b sync_el1x
    .align 7; b irq_el1x
    .align 7; b fiq_el1x
    .align 7; b serr_el1x

    /* Lower EL using AArch64 */
    .align 7; b sync_el0_64
    .align 7; b irq_el0_64
    .align 7; b fiq_el0_64
    .align 7; b serr_el0_64

    /* Lower EL using AArch32 */
    .align 7; b sync_el0_32
    .align 7; b irq_el0_32
    .align 7; b fiq_el0_32
    .align 7; b serr_el0_32

/* ── Save / restore context ──────────────────────────────────────── */

.macro save_context
    sub sp, sp, #272
    stp x0, x1, [sp, #0]
    stp x2, x3, [sp, #16]
    stp x4, x5, [sp, #32]
    stp x6, x7, [sp, #48]
    stp x8, x9, [sp, #64]
    stp x10, x11, [sp, #80]
    stp x12, x13, [sp, #96]
    stp x14, x15, [sp, #112]
    stp x16, x17, [sp, #128]
    stp x18, x19, [sp, #144]
    stp x20, x21, [sp, #160]
    stp x22, x23, [sp, #176]
    stp x24, x25, [sp, #192]
    stp x26, x27, [sp, #208]
    stp x28, x29, [sp, #224]
    str x30, [sp, #240]
    mrs x0, spsr_el1
    mrs x1, elr_el1
    stp x0, x1, [sp, #248]
    mrs x0, esr_el1
    mrs x1, far_el1
    stp x0, x1, [sp, #264]
.endm

.macro restore_context
    ldp x0, x1, [sp, #248]
    msr spsr_el1, x0
    msr elr_el1, x1
    ldp x0, x1, [sp, #264]
    msr esr_el1, x0
    msr far_el1, x1
    ldp x28, x29, [sp, #224]
    ldr x30, [sp, #240]
    ldp x26, x27, [sp, #208]
    ldp x24, x25, [sp, #192]
    ldp x22, x23, [sp, #176]
    ldp x20, x21, [sp, #160]
    ldp x18, x19, [sp, #144]
    ldp x16, x17, [sp, #128]
    ldp x14, x15, [sp, #112]
    ldp x12, x13, [sp, #96]
    ldp x10, x11, [sp, #80]
    ldp x8, x9, [sp, #64]
    ldp x6, x7, [sp, #48]
    ldp x4, x5, [sp, #32]
    ldp x2, x3, [sp, #16]
    ldp x0, x1, [sp, #0]
    add sp, sp, #272
.endm

/* ── Exception entry points ──────────────────────────────────────── */

sync_el1t_sp0:
sync_el1x:
    save_context
    mov x0, #0   /* exception type: sync EL1 */
    b exception_handler_common

irq_el1t_sp0:
irq_el1x:
    save_context
    mov x0, #1   /* IRQ EL1 */
    b exception_handler_common

fiq_el1t_sp0:
fiq_el1x:
    save_context
    mov x0, #2   /* FIQ EL1 */
    b exception_handler_common

serr_el1t_sp0:
serr_el1x:
    save_context
    mov x0, #3   /* SError EL1 */
    b exception_handler_common

/* ── SVC from EL0 (system call) ──────────────────────────────────── */

sync_el0_64:
    save_context
    mov x0, #4   /* SVC from EL0 */
    mrs x1, esr_el1
    and w1, w1, #0xFFFF  /* ESR_EL1 exception class (24:26) + ISS (0:23) */
    mov x2, sp
    b exception_handler_common

irq_el0_64:
    save_context
    mov x0, #5   /* IRQ from EL0 */
    b exception_handler_common

fiq_el0_64:
sync_el0_32:
irq_el0_32:
fiq_el0_32:
serr_el0_64:
serr_el0_32:
    save_context
    mov x0, #6
    b exception_handler_common

/* ── C handler call ──────────────────────────────────────────────── */

exception_handler_common:
    /* x0 = type, x1 = esr, x2 = sp (saved context) */
    mov x29, x2
    mov x2, x29
    bl arm64_exception_handler
    /* After handler returns, restore and eret */
    restore_context
    eret

/* ── ERET to EL0 (enter userspace) ───────────────────────────────── */

.global arm64_enter_el0
arm64_enter_el0:
    /* x0 = entry point, x1 = stack pointer, x2 = ttbr0 (user page table) */
    msr ttbr0_el1, x2
    isb
    msr sp_el0, x1
    msr elr_el1, x0
    mov x4, #0x3C0   /* SPSR: EL0t, all interrupts enabled */
    msr spsr_el1, x4
    eret
