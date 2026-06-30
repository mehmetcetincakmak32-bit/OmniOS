/* OmniOS — kernel/arch/x86_64/interrupts.c */
/* IRQ dispatch — PIC remap + APIC-based interrupt routing */
/* SPDX-License-Identifier: MIT */

#include "interrupts.h"
#include "apic.h"
#include "io.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

#define IRQ_MAX 256

static irq_handler_t irq_handlers[IRQ_MAX];
static uint64_t irq_counts[IRQ_MAX];

/* ── PIC (legacy) remap to avoid reserved vectors ────────────────── */

static void pic_remap(void) {
    outb(0x20, 0x11); outb(0xA0, 0x11);  /* ICW1 */
    outb(0x21, 0x20); outb(0xA1, 0x28);  /* ICW2: master offset 32, slave 40 */
    outb(0x21, 0x04); outb(0xA1, 0x02);  /* ICW3: cascade */
    outb(0x21, 0x01); outb(0xA1, 0x01);  /* ICW4 */
    outb(0x21, 0xFF); outb(0xA1, 0xFF);  /* Mask all */
    io_wait();
}

/* ── Generic IRQ handler (called from assembly stub) ─────────────── */

void irq_dispatch(interrupt_frame_t *frame) {
    uint8_t vec = frame->vector;
    irq_counts[vec]++;

    if (vec >= IRQ_BASE && vec < IRQ_BASE + 16) {
        /* Legacy PIC IRQ */
        irq_handler_t h = irq_handlers[vec - IRQ_BASE];
        if (h) h(vec - IRQ_BASE, frame);
        irq_end_of_interrupt(vec - IRQ_BASE);
        return;
    }

    /* APIC interrupt */
    irq_handler_t h = irq_handlers[vec];
    if (h) h(vec, frame);

    apic_eoi();
}

void irq_end_of_interrupt(uint8_t irq) {
    if (irq >= 8) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

/* ── Init interrupts ─────────────────────────────────────────────── */

void irq_init(void) {
    memset(irq_handlers, 0, sizeof(irq_handlers));
    memset(irq_counts, 0, sizeof(irq_counts));

    pic_remap();
    printf("[IRQ] PIC remapped (master=32, slave=40)\n");
}

void irq_register(uint8_t irq, irq_handler_t handler) {
    if (irq >= IRQ_MAX) return;
    irq_handlers[irq] = handler;

    /* Unmask in PIC for legacy IRQs */
    if (irq < 16) {
        if (irq < 8) {
            outb(0x21, inb(0x21) & ~(1 << irq));
        } else {
            outb(0xA1, inb(0xA1) & ~(1 << (irq - 8)));
        }
        /* Redirect via I/O APIC if available */
        ioapic_redirect_irq(irq, IRQ_BASE + irq, 0);
    }
}

void irq_unregister(uint8_t irq) {
    if (irq >= IRQ_MAX) return;
    irq_handlers[irq] = NULL;
    if (irq < 16) {
        if (irq < 8) outb(0x21, inb(0x21) | (1 << irq));
        else outb(0xA1, inb(0xA1) | (1 << (irq - 8)));
    }
}

void irq_enable(void) { __asm__ volatile("sti"); }
void irq_disable(void) { __asm__ volatile("cli"); }
