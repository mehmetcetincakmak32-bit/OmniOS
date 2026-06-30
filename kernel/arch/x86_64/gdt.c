/* OmniOS — kernel/arch/x86_64/gdt.c */
/* x86_64 GDT + TSS per-CPU */
/* SPDX-License-Identifier: MIT */

#include "gdt.h"
#include <stdint.h>
#include <string.h>

/* ── GDT descriptor (8 bytes) ────────────────────────────────────── */

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  flags_limit;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdt_ptr_t;

/* ── TSS descriptor (16 bytes, spans 2 GDT entries) ──────────────── */

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  flags_limit;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed)) tss_descriptor_t;

/* ── Task State Segment (per-CPU) ────────────────────────────────── */

typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;          /* Kernel stack for ring 0 */
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];        /* Interrupt Stack Table */
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed)) tss_t;

_Static_assert(sizeof(tss_t) == 104, "TSS size mismatch");

/* ── Global state ────────────────────────────────────────────────── */

static gdt_entry_t gdt[GDT_ENTRIES];
static tss_t tss_entries[MAX_CPUS];

/* ── GDT entry builder ───────────────────────────────────────────── */

static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
        uint8_t access, uint8_t flags) {
    gdt[i].limit_low  = limit & 0xFFFF;
    gdt[i].base_low   = base & 0xFFFF;
    gdt[i].base_mid   = (base >> 16) & 0xFF;
    gdt[i].access     = access;
    gdt[i].flags_limit = ((limit >> 16) & 0x0F) | (flags & 0xF0);
    gdt[i].base_high  = (base >> 24) & 0xFF;
}

/* ── TSS descriptor in GDT ───────────────────────────────────────── */

static void gdt_set_tss_entry(int i, uintptr_t tss_addr, size_t tss_size) {
    uint32_t base = (uint32_t)(tss_addr & 0xFFFFFFFF);
    uint32_t limit = (uint32_t)(tss_size - 1);

    /* First descriptor (low) */
    gdt[i].limit_low  = limit & 0xFFFF;
    gdt[i].base_low   = base & 0xFFFF;
    gdt[i].base_mid   = (base >> 16) & 0xFF;
    gdt[i].access     = 0x89;  /* Present, 64-bit TSS (type 1001) */
    gdt[i].flags_limit = (limit >> 16) & 0x0F;
    gdt[i].base_high  = (base >> 24) & 0xFF;

    /* Second descriptor (high 32 bits + reserved) */
    uint64_t base_upper = (uint64_t)(tss_addr >> 32);
    uint32_t *hi = (uint32_t *)&gdt[i + 1];
    hi[0] = (uint32_t)(base_upper & 0xFFFFFFFF);
    hi[1] = 0;
}

/* ── Initialize GDT ──────────────────────────────────────────────── */

void gdt_init(void) {
    memset(gdt, 0, sizeof(gdt));

    /* Null descriptor */
    gdt_set_entry(0, 0, 0, 0, 0);
    /* Kernel code (ring 0, 64-bit) */
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xA0);
    /* Kernel data (ring 0) */
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xA0);
    /* User code (ring 3, 64-bit) */
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xA0);
    /* User data (ring 3) */
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xA0);

    /* Bootstrap TSS at BSP index (0) */
    uint32_t bsp_idx = 0;
    tss_t *tss_bsp = &tss_entries[bsp_idx];
    memset(tss_bsp, 0, sizeof(*tss_bsp));
    tss_bsp->iomap_base = sizeof(*tss_bsp);
    gdt_set_tss_entry(GDT_TSS_BASE / 8, (uintptr_t)tss_bsp, sizeof(*tss_bsp));

    /* Load GDT and TSS */
    gdt_ptr_t ptr;
    ptr.limit = sizeof(gdt) - 1;
    ptr.base  = (uint64_t)&gdt;
    __asm__ volatile("lgdt %0" : : "m"(ptr));

    __asm__ volatile("movw %0, %%ax; ltr %%ax"
        : : "r"((uint16_t)GDT_TSS_BASE));
}

/* ── Load TSS for a given CPU (for SMP) ──────────────────────────── */

void gdt_load_tss(uint32_t cpu_index, uintptr_t tss_addr, size_t tss_size) {
    if (cpu_index >= MAX_CPUS) return;
    int entry = GDT_TSS_BASE / 8 + cpu_index * 2;
    if (entry + 1 >= GDT_ENTRIES) return;

    tss_t *tss = &tss_entries[cpu_index];
    memset(tss, 0, sizeof(*tss));
    tss->iomap_base = sizeof(*tss);
    if (tss_addr) {
        tss->rsp0 = tss_addr + tss_size;
    }

    gdt_set_tss_entry(entry, (uintptr_t)tss, sizeof(*tss));

    /* Load TR for this CPU */
    uint16_t sel = (uint16_t)(entry * 8);
    /* Note: ltr only works once per CPU; for APs, call after switching to per-CPU GDT */
}
