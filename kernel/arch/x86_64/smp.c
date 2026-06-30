/* OmniOS — kernel/arch/x86_64/smp.c */
/* SMP boot protocol — bring up APs via SIPI */
/* SPDX-License-Identifier: MIT */

#include "smp.h"
#include "apic.h"
#include "cpu.h"
#include "io.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

/* AP trampoline boundaries (defined in boot.S) */
extern uint8_t ap_trampoline_start[];
extern uint8_t ap_trampoline_end[];

static volatile int online_cpus = 1;
static volatile int ap_ready_count = 0;
static volatile uint32_t ap_stacks[MAX_CPUS];

/* Per-CPU control blocks */
static percpu_t percpu_blocks[MAX_CPUS];
static int cpu_index_map[MAX_CPUS]; /* APIC ID → cpu_index */

uint32_t smp_current_cpu(void) {
    uint32_t apic_id = apic_get_id();
    for (int i = 0; i < MAX_CPUS; i++) {
        if (cpu_index_map[i] == (int)apic_id) return i;
    }
    return 0;
}

int smp_online_cpus(void) {
    return online_cpus;
}

/* ── AP entry point (called from trampoline) ─────────────────────── */

void ap_entry(uint32_t apic_id) {
    uint32_t cpu_index = online_cpus++;

    percpu_blocks[cpu_index].apic_id = apic_id;
    percpu_blocks[cpu_index].cpu_index = cpu_index;
    percpu_blocks[cpu_index].online = true;
    percpu_blocks[cpu_index].boot_cpu = false;

    /* Set GS.base to per-CPU block */
    uintptr_t percpu_addr = (uintptr_t)&percpu_blocks[cpu_index];
    __asm__ volatile("wrmsr" :
        : "a"((uint32_t)(percpu_addr & 0xFFFFFFFF)),
          "d"((uint32_t)(percpu_addr >> 32)),
          "c"(0xC0000101));  /* MSR_GS_BASE */

    /* Initialize local APIC for this CPU */
    apic_init();

    /* Set up per-CPU TSS */
    cpu_index_map[cpu_index] = apic_id;

    __atomic_fetch_add(&ap_ready_count, 1, __ATOMIC_SEQ_CST);

    printf("[SMP] AP %u online (APIC %u)\n", cpu_index, apic_id);

    /* Enable interrupts and idle */
    __asm__ volatile("sti");
    while (1) { __asm__ volatile("hlt"); }
}

/* ── Bootstrap APs ───────────────────────────────────────────────── */

void smp_boot_aps(void) {
    /* Copy AP trampoline to low memory */
    size_t tramp_size = ap_trampoline_end - ap_trampoline_start;
    if (tramp_size > 0x1000) {
        printf("[SMP] Trampoline too large (%zu bytes)\n", tramp_size);
        return;
    }
    memcpy((void *)AP_TRAMPOLINE_ADDR, ap_trampoline_start, tramp_size);

    printf("[SMP] Booting %d APs...\n", MAX_CPUS - 1);

    /* Send INIT IPI */
    apic_send_ipi_all(0);
    io_wait();
    __asm__ volatile("pause");

    /* Send STARTUP IPI */
    apic_send_ipi_all(AP_TRAMPOLINE_ADDR / PAGE_SIZE);
    io_wait();

    /* Wait for APs to come online */
    for (int i = 0; i < 100; i++) {
        if (ap_ready_count >= MAX_CPUS - 1) break;
        /* Send another SIPI */
        apic_send_ipi_all(AP_TRAMPOLINE_ADDR / PAGE_SIZE);
        for (volatile int j = 0; j < 100000; j++) __asm__ volatile("pause");
    }

    printf("[SMP] %d CPUs online\n", online_cpus);
}

/* ── Send IPI ────────────────────────────────────────────────────── */

void smp_send_ipi(uint32_t cpu, uint8_t vector) {
    if (cpu < MAX_CPUS) {
        apic_send_ipi(cpu_index_map[cpu], vector);
    }
}

void smp_send_broadcast(uint8_t vector) {
    apic_send_ipi_all_except_self(vector);
}

/* ── Per-CPU init (BSP) ──────────────────────────────────────────── */

void smp_percpu_init(uint32_t cpu_index) {
    uint32_t apic_id = apic_get_id();

    percpu_blocks[cpu_index].apic_id = apic_id;
    percpu_blocks[cpu_index].cpu_index = cpu_index;
    percpu_blocks[cpu_index].online = true;
    percpu_blocks[cpu_index].boot_cpu = true;

    /* Set GS.base to per-CPU block */
    uintptr_t percpu_addr = (uintptr_t)&percpu_blocks[cpu_index];
    __asm__ volatile("wrmsr" :
        : "a"((uint32_t)(percpu_addr & 0xFFFFFFFF)),
          "d"((uint32_t)(percpu_addr >> 32)),
          "c"(0xC0000101));

    cpu_index_map[cpu_index] = apic_id;
    printf("[SMP] BSP initialized (cpu %d, APIC %u)\n", cpu_index, apic_id);
}

/* ─── SMP init ───────────────────────────────────────────────────── */

void smp_init(void) {
    memset(cpu_index_map, 0xFF, sizeof(cpu_index_map));
    smp_percpu_init(0);
    smp_boot_aps();
}
