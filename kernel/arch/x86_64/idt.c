/* OmniOS — kernel/arch/x86_64/idt.c */
/* x86_64 IDT + exception stubs + dispatch */
/* SPDX-License-Identifier: MIT */

#include "idt.h"
#include <string.h>
#include <stdio.h>

static idt_entry_t idt[IDT_ENTRIES];
static interrupt_handler_t handlers[IDT_ENTRIES];

const char *exception_names[] = {
    [0]  = "Divide-by-zero",
    [1]  = "Debug",
    [2]  = "Non-maskable Interrupt",
    [3]  = "Breakpoint",
    [4]  = "Overflow",
    [5]  = "Bound Range Exceeded",
    [6]  = "Invalid Opcode",
    [7]  = "Device Not Available",
    [8]  = "Double Fault",
    [9]  = "Coprocessor Segment Overrun",
    [10] = "Invalid TSS",
    [11] = "Segment Not Present",
    [12] = "Stack-Segment Fault",
    [13] = "General Protection Fault",
    [14] = "Page Fault",
    [15] = "Reserved",
    [16] = "x87 FPU Floating-Point Error",
    [17] = "Alignment Check",
    [18] = "Machine Check",
    [19] = "SIMD Floating-Point Exception",
    [20] = "Virtualization Exception",
    [21] = "Control Protection Exception",
};

/* ── IDT entry builder ───────────────────────────────────────────── */

void idt_set_handler(uint8_t vec, interrupt_handler_t handler,
        uint8_t ist, bool user) {
    uint64_t addr = (uint64_t)handler;
    idt[vec].offset_low   = addr & 0xFFFF;
    idt[vec].selector     = 0x08;  /* kernel code segment */
    idt[vec].ist          = ist;
    idt[vec].type_attr    = 0x8E | (user ? 0x60 : 0);  /* present, interrupt gate */
    idt[vec].offset_mid   = (addr >> 16) & 0xFFFF;
    idt[vec].offset_high  = (addr >> 32) & 0xFFFFFFFF;
    idt[vec].reserved     = 0;
    handlers[vec] = handler;
}

/* ── Default handler (panic on exception) ────────────────────────── */

static void default_handler(interrupt_frame_t *frame) {
    const char *name = "Unknown";
    if (frame->vector < sizeof(exception_names)/sizeof(exception_names[0])
        && exception_names[frame->vector]) {
        name = exception_names[frame->vector];
    }

    printf("\n=== OmniOS EXCEPTION ===\n");
    printf("  Vector: %u (%s)\n", frame->vector, name);
    printf("  Error:  0x%llX\n", (unsigned long long)frame->error_code);
    printf("  RIP:    0x%llX\n", (unsigned long long)frame->rip);
    printf("  RSP:    0x%llX\n", (unsigned long long)frame->rsp);
    printf("  CS:     0x%llX\n", (unsigned long long)frame->cs);
    printf("  RFLAGS: 0x%llX\n", (unsigned long long)frame->rflags);
    printf("  RAX:    0x%llX  RBX: 0x%llX\n",
        (unsigned long long)frame->rax, (unsigned long long)frame->rbx);
    printf("  RCX:    0x%llX  RDX: 0x%llX\n",
        (unsigned long long)frame->rcx, (unsigned long long)frame->rdx);
    printf("  RSI:    0x%llX  RDI: 0x%llX\n",
        (unsigned long long)frame->rsi, (unsigned long long)frame->rdi);
    printf("  RBP:    0x%llX  R8:  0x%llX\n",
        (unsigned long long)frame->rbp, (unsigned long long)frame->r8);

    if (frame->vector == 14) {
        uint64_t cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
        printf("  CR2:    0x%llX (fault address)\n", (unsigned long long)cr2);
    }

    printf("=== SYSTEM HALTED ===\n");
    while (1) { __asm__ volatile("cli; hlt"); }
}

/* ── Install IDT ─────────────────────────────────────────────────── */

void idt_install(void) {
    idt_ptr_t ptr;
    ptr.limit = sizeof(idt) - 1;
    ptr.base  = (uint64_t)&idt;
    __asm__ volatile("lidt %0" : : "m"(ptr));
}

/* ── Initialize IDT ──────────────────────────────────────────────── */

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    memset(handlers, 0, sizeof(handlers));

    /* Set default handlers for all exception vectors */
    for (int i = 0; i < 32; i++) {
        uint8_t ist = 0;
        if (i == 8)  ist = IST_DOUBLE_FAULT;  /* Double fault -> IST 1 */
        if (i == 2)  ist = IST_NMI;            /* NMI -> IST 2 */
        if (i == 18) ist = IST_MCE;            /* MCE -> IST 3 */
        idt_set_handler(i, default_handler, ist, false);
    }

    idt_install();
    printf("[IDT] 256 vector IDT installed\n");
}
