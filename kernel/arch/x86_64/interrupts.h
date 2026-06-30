/* OmniOS — kernel/arch/x86_64/interrupts.h */
/* IRQ dispatch — PIC, APIC, spurious, EOI */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_INTERRUPTS_H
#define OMNIOS_INTERRUPTS_H

#include <stdint.h>
#include "idt.h"

/* IRQ handler type */
typedef void (*irq_handler_t)(uint8_t irq, interrupt_frame_t *frame);

void irq_init(void);
void irq_register(uint8_t irq, irq_handler_t handler);
void irq_unregister(uint8_t irq);
void irq_enable(void);
void irq_disable(void);
void irq_end_of_interrupt(uint8_t irq);

/* Default handlers for x86_64 devices */
void irq_handler_pit(interrupt_frame_t *frame);
void irq_handler_keyboard(interrupt_frame_t *frame);
void irq_handler_spurious(interrupt_frame_t *frame);

#endif /* OMNIOS_INTERRUPTS_H */
