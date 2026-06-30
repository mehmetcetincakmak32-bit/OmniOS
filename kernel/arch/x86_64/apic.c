/* OmniOS — kernel/arch/x86_64/apic.c */
/* APIC + I/O APIC implementation */
/* SPDX-License-Identifier: MIT */

#include "apic.h"
#include "io.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>

static volatile void *lapic_base = NULL;
static bool x2apic_mode = false;
static uint32_t lapic_freq_hz = 0;

/* MSR read/write */
static inline uint64_t read_msr(uint32_t msr) {
    uint32_t lo, hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}
static inline void write_msr(uint32_t msr, uint64_t val) {
    __asm__ volatile("wrmsr" : : "a"((uint32_t)val), "d"((uint32_t)(val >> 32)), "c"(msr));
}

static uint32_t lapic_read(uint32_t reg) {
    if (x2apic_mode) return (uint32_t)read_msr(0x800 + reg / 16);
    return mmio_read32(lapic_base + reg);
}

static void lapic_write(uint32_t reg, uint32_t val) {
    if (x2apic_mode) { write_msr(0x800 + reg / 16, val); return; }
    mmio_write32(lapic_base + reg, val);
}

bool apic_is_x2apic(void) { return x2apic_mode; }

uint32_t apic_get_id(void) {
    uint32_t id = lapic_read(LAPIC_ID_REG);
    if (x2apic_mode) return id >> 24;
    return id >> 24;
}

void apic_init(void) {
    uint64_t apic_base_msr = read_msr(MSR_APIC_BASE);

    /* Check x2APIC enable */
    if (apic_base_msr & (1 << 10)) {
        x2apic_mode = true;
        printf("[APIC] x2APIC mode\n");
    } else {
        lapic_base = (volatile void *)(uintptr_t)(apic_base_msr & 0xFFFFFF000ULL);
        printf("[APIC] xAPIC mode @ %p\n", lapic_base);
    }

    /* Enable spurious vector */
    lapic_write(LAPIC_SPURIOUS, 0xFF | LAPIC_SPURIOUS_ENABLE | LAPIC_SPURIOUS_FOCUS);

    /* Accept all interrupts */
    lapic_write(LAPIC_TPR, 0);

    /* Clear error */
    lapic_write(LAPIC_ESR, 0);
    (void)lapic_read(LAPIC_ESR);

    uint32_t ver = lapic_read(LAPIC_VER_REG);
    printf("[APIC] Local APIC version 0x%x, max LVT: %u\n",
        ver & 0xFF, (ver >> 16) & 0xFF);

    /* Mask legacy LINT0/LINT1 in xAPIC mode */
    if (!x2apic_mode) {
        lapic_write(LAPIC_LVT_LINT0, LAPIC_LVT_MASKED);
        lapic_write(LAPIC_LVT_LINT1, LAPIC_LVT_MASKED);
    }
    lapic_write(LAPIC_LVT_ERROR, LAPIC_LVT_DM_FIXED | (0x37));

    /* Calibrate APIC timer against PIT */
    uint32_t initial = 0xFFFFFFFF;
    lapic_write(LAPIC_TIMER_DIV, 3); /* divide by 16 */
    lapic_write(LAPIC_TIMER_INIT, initial);
    lapic_write(LAPIC_LVT_TIMER, LAPIC_TIMER_ONESHOT | LAPIC_LVT_MASKED);

    /* PIT: wait 10ms */
    outb(0x43, 0x30);
    outb(0x40, 0x9C);
    outb(0x40, 0x2E);
    uint32_t start = inb(0x40) | (inb(0x40) << 8);
    for (volatile int i = 0; i < 100000; i++) __asm__ volatile("pause");
    uint32_t end = inb(0x40) | (inb(0x40) << 8);

    uint32_t elapsed = lapic_read(LAPIC_TIMER_CUR);
    uint32_t pit_ticks = (uint16_t)(start - end);
    if (pit_ticks > 0) {
        lapic_freq_hz = (initial - elapsed) * 100 / (pit_ticks * 16);
        printf("[APIC] Timer freq: ~%u Hz\n", lapic_freq_hz);
    }
}

void apic_eoi(void) {
    if (x2apic_mode) write_msr(MSR_X2APIC_EOI, 0);
    else lapic_write(LAPIC_EOI, 0);
}

void apic_send_ipi(uint32_t apic_id, uint8_t vector) {
    if (x2apic_mode) {
        write_msr(MSR_X2APIC_ICR, ((uint64_t)apic_id << 32) | vector);
    } else {
        lapic_write(LAPIC_ICR_HIGH, apic_id << 24);
        lapic_write(LAPIC_ICR_LOW, vector);
        while (lapic_read(LAPIC_ICR_LOW) & (1 << 12));
    }
}

void apic_send_ipi_all(uint8_t vector) {
    if (x2apic_mode) {
        write_msr(MSR_X2APIC_ICR, ((uint64_t)LAPIC_ICR_ALL << 18) | vector);
    } else {
        lapic_write(LAPIC_ICR_HIGH, 0);
        lapic_write(LAPIC_ICR_LOW, LAPIC_ICR_ALL | vector);
        while (lapic_read(LAPIC_ICR_LOW) & (1 << 12));
    }
}

void apic_send_ipi_all_except_self(uint8_t vector) {
    if (x2apic_mode) {
        write_msr(MSR_X2APIC_ICR, ((uint64_t)LAPIC_ICR_ALL_EXCL_SELF << 18) | vector);
    } else {
        lapic_write(LAPIC_ICR_HIGH, 0);
        lapic_write(LAPIC_ICR_LOW, LAPIC_ICR_ALL_EXCL_SELF | vector);
        while (lapic_read(LAPIC_ICR_LOW) & (1 << 12));
    }
}

void apic_timer_start(uint32_t ticks, uint8_t vector, bool periodic) {
    lapic_write(LAPIC_TIMER_DIV, 3);
    lapic_write(LAPIC_TIMER_INIT, ticks);
    uint32_t mode = periodic ? LAPIC_TIMER_PERIODIC : LAPIC_TIMER_ONESHOT;
    lapic_write(LAPIC_LVT_TIMER, mode | vector);
}

void apic_timer_stop(void) {
    lapic_write(LAPIC_LVT_TIMER, LAPIC_LVT_MASKED);
    lapic_write(LAPIC_TIMER_INIT, 0);
}

/* I/O APIC */

#define IOAPIC_ADDR 0xFEC00000

static volatile void *ioapic_base = NULL;

static uint32_t ioapic_read(uint8_t reg) {
    mmio_write32(ioapic_base + IOAPIC_IOREGSEL, reg);
    return mmio_read32(ioapic_base + IOAPIC_IOWIN);
}

static void ioapic_write(uint8_t reg, uint32_t val) {
    mmio_write32(ioapic_base + IOAPIC_IOREGSEL, reg);
    mmio_write32(ioapic_base + IOAPIC_IOWIN, val);
}

void ioapic_init(void) {
    ioapic_base = (volatile void *)(uintptr_t)IOAPIC_ADDR;
    uint32_t id = ioapic_read(IOAPIC_ID);
    uint32_t ver = ioapic_read(IOAPIC_VER);
    uint32_t max_redir = (ver >> 16) & 0xFF;
    printf("[IOAPIC] ID=%u version=0x%x max_redir=%u\n", id >> 24, ver & 0xFF, max_redir);

    /* Mask all redirection entries */
    for (uint32_t i = 0; i <= max_redir; i++) {
        ioapic_write(IOAPIC_REDIR_TBL + i * 2, 1 << 16);
        ioapic_write(IOAPIC_REDIR_TBL + i * 2 + 1, 0);
    }
}

void ioapic_redirect_irq(uint8_t irq, uint8_t vector, uint32_t apic_id) {
    uint32_t idx = IOAPIC_REDIR_TBL + irq * 2;
    uint32_t low = vector | (apic_id << 0);
    uint32_t high = apic_id << 24;
    ioapic_write(idx, low);
    ioapic_write(idx + 1, high);
}
