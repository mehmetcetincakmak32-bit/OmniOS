/* OmniOS — kernel/arch/x86_64/apic.h */
/* x86_64 Local APIC + I/O APIC (xAPIC / x2APIC) for SMP */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_APIC_H
#define OMNIOS_APIC_H

#include <stdint.h>
#include <stdbool.h>

#define LAPIC_ID_REG        0x020
#define LAPIC_VER_REG       0x030
#define LAPIC_TPR           0x080
#define LAPIC_APR           0x090
#define LAPIC_PPR           0x0A0
#define LAPIC_EOI           0x0B0
#define LAPIC_LDR           0x0D0
#define LAPIC_DFR           0x0E0
#define LAPIC_SPURIOUS      0x0F0
#define LAPIC_ISR_BASE      0x100
#define LAPIC_TMR_BASE      0x180
#define LAPIC_IRR_BASE      0x200
#define LAPIC_ESR           0x280
#define LAPIC_ICR_LOW       0x300
#define LAPIC_ICR_HIGH      0x310
#define LAPIC_LVT_TIMER     0x320
#define LAPIC_LVT_THERMAL   0x330
#define LAPIC_LVT_PERFMON   0x340
#define LAPIC_LVT_LINT0     0x350
#define LAPIC_LVT_LINT1     0x360
#define LAPIC_LVT_ERROR     0x370
#define LAPIC_TIMER_INIT    0x380
#define LAPIC_TIMER_CUR     0x390
#define LAPIC_TIMER_DIV     0x3E0

#define LAPIC_SPURIOUS_ENABLE   (1 << 8)
#define LAPIC_SPURIOUS_FOCUS    (1 << 9)
#define LAPIC_ICR_INIT          (5 << 8)
#define LAPIC_ICR_STARTUP       (6 << 8)
#define LAPIC_ICR_LEVEL_ASSERT  (1 << 14)
#define LAPIC_ICR_LEVEL_DEASSERT (0 << 14)
#define LAPIC_ICR_TRIGGER_LEVEL (1 << 15)
#define LAPIC_ICR_DM_LOGICAL    (1 << 11)
#define LAPIC_ICR_DM_PHYSICAL   (0 << 11)
#define LAPIC_ICR_ALL_EXCL_SELF (3 << 18)
#define LAPIC_ICR_ALL           (2 << 18)
#define LAPIC_LVT_MASKED        (1 << 16)
#define LAPIC_LVT_TRIGGER_LEVEL (1 << 13)
#define LAPIC_LVT_REMOTE_IRR    (1 << 14)
#define LAPIC_LVT_DM_FIXED      0
#define LAPIC_LVT_DM_NMI        0x400
#define LAPIC_LVT_DM_SMI        0x800
#define LAPIC_LVT_DM_INIT       0xC00
#define LAPIC_LVT_DM_EXTINT     0x700
#define LAPIC_TIMER_ONESHOT     0
#define LAPIC_TIMER_PERIODIC    (1 << 17)
#define LAPIC_TIMER_TSC_DEADLINE (1 << 18)
#define MSR_APIC_BASE          0x01B
#define MSR_X2APIC_ICR         0x830
#define MSR_X2APIC_EOI         0x80B
#define MSR_X2APIC_SELF_IPI    0x83F
#define IOAPIC_IOREGSEL        0x00
#define IOAPIC_IOWIN           0x10
#define IOAPIC_ID              0x00
#define IOAPIC_VER             0x01
#define IOAPIC_ARB             0x02
#define IOAPIC_REDIR_TBL       0x10

void apic_init(void);
void apic_eoi(void);
void apic_send_ipi(uint32_t apic_id, uint8_t vector);
void apic_send_ipi_all(uint8_t vector);
void apic_send_ipi_all_except_self(uint8_t vector);
void apic_timer_start(uint32_t ticks, uint8_t vector, bool periodic);
void apic_timer_stop(void);
uint32_t apic_get_id(void);
bool apic_is_x2apic(void);
void ioapic_init(void);
void ioapic_redirect_irq(uint8_t irq, uint8_t vector, uint32_t apic_id);

#endif /* OMNIOS_APIC_H */
