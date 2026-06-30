/* OmniOS — kernel/arch/arm64/arm64.c */
/* ARM64 GICv3, timer, paging, CPU init, page tables */
/* SPDX-License-Identifier: MIT */

#include "arm64.h"
#include "../../mm/pmm.h"
#include <stdio.h>
#include <string.h>

/* Page table descriptor manipulation */
#define PT_VALID      (1ULL << 0)
#define PT_TABLE      (1ULL << 1)
#define PT_BLOCK      (1ULL << 1)
#define PT_AF         (1ULL << 10)
#define PT_AP_RW_ALL  (0ULL << 6)   /* EL0/EL1 RW */
#define PT_AP_RO_ALL  (1ULL << 6)   /* EL0/EL1 RO */
#define PT_AP_RW_EL1  (2ULL << 6)   /* EL1 only RW */
#define PT_NS         (1ULL << 5)
#define PT_ATTR_INDX  (0ULL << 2)
#define PT_PXN        (1ULL << 53)
#define PT_UXN        (1ULL << 54)
#define PT_ADDR_MASK  0x0000FFFFFFFFF000ULL

#define L0_SHIFT  39
#define L1_SHIFT  30
#define L2_SHIFT  21
#define L3_SHIFT  12
#define Lx_SIZE(x) (1ULL << (x))
#define Lx_MASK(x) ((Lx_SIZE(x)) - 1)
#define Lx_INDEX(virt, shift, level) (((virt) >> (shift)) & 0x1FF)

/* Current kernel TTBR1 page table base */
static uintptr_t kernel_pgd = 0;

/* ── GIC ─────────────────────────────────────────────────────────── */

void arm64_gic_init(void) {
    gic_write32(GICD_BASE, GICD_CTLR, 1);
    gic_write32(GICC_BASE, GICC_CTLR, 1);
    gic_write32(GICC_BASE, GICC_PMR, 0xFF);
    uint32_t typer = gic_read32(GICD_BASE, GICD_TYPER);
    printf("[GIC] GICv3: %d IRQ lines\n", ((typer >> 0) & 0x1F) + 1);
}

void arm64_gic_eoi(uint32_t irq) {
    gic_write32(GICC_BASE, GICC_EOIR, irq);
}

/* ── Generic Timer ────────────────────────────────────────────────── */

void arm64_timer_init(uint32_t freq_hz) {
    (void)freq_hz;
    uint64_t freq = arm64_read_cntfrq();
    printf("[TIMER] %llu Hz\n", (unsigned long long)freq);
    arm64_write_cntp_tval(freq / 100);
    arm64_write_cntp_ctl(1);
}

/* ── Page table helpers ──────────────────────────────────────────── */

static uintptr_t pt_lookup_or_create(uintptr_t *table, int index, int level) {
    if (table[index] & PT_VALID) {
        return table[index] & PT_ADDR_MASK;
    }
    uintptr_t pg = pmm_alloc_page();
    memset((void *)pg, 0, PAGE_SIZE);
    table[index] = pg | PT_VALID | PT_TABLE | PT_AF | PT_NS;
    return pg;
}

static void pt_map_page(uintptr_t pgd, uintptr_t virt, uintptr_t phys, uint64_t flags) {
    int l0 = Lx_INDEX(virt, L0_SHIFT, 0);
    int l1 = Lx_INDEX(virt, L1_SHIFT, 1);
    int l2 = Lx_INDEX(virt, L2_SHIFT, 2);
    int l3 = Lx_INDEX(virt, L3_SHIFT, 3);

    uintptr_t *t0 = (uintptr_t *)pgd;
    uintptr_t t1_pa = pt_lookup_or_create(t0, l0, 0);
    uintptr_t *t1 = (uintptr_t *)t1_pa;
    uintptr_t t2_pa = pt_lookup_or_create(t1, l1, 1);
    uintptr_t *t2 = (uintptr_t *)t2_pa;
    uintptr_t t3_pa = pt_lookup_or_create(t2, l2, 2);
    uintptr_t *t3 = (uintptr_t *)t3_pa;

    /* Set page: valid + AF + attributes */
    t3[l3] = (phys & PT_ADDR_MASK) | PT_VALID | PT_AF | PT_NS | flags;
    __asm__ volatile("dsb ishst");
}

uintptr_t arm64_create_user_pgd(void) {
    uintptr_t pgd = pmm_alloc_page();
    memset((void *)pgd, 0, PAGE_SIZE);
    printf("[PAGING] User PGD at 0x%lx\n", pgd);
    return pgd;
}

void arm64_map_user_page(uintptr_t pgd, uintptr_t virt, uintptr_t phys, int writable, int executable) {
    uint64_t flags = PT_AP_RW_ALL;
    if (!writable) flags = PT_AP_RO_ALL;
    if (!executable) flags |= PT_UXN;  /* User Execute Never */
    pt_map_page(pgd, virt, phys, flags);
}

void arm64_map_user_pages(uintptr_t pgd, uintptr_t virt, uintptr_t phys, size_t pages, int writable, int executable) {
    for (size_t i = 0; i < pages; i++) {
        arm64_map_user_page(pgd, virt + i * PAGE_SIZE, phys + i * PAGE_SIZE, writable, executable);
    }
}

void arm64_set_ttbr0(uintptr_t pgd) {
    __asm__ volatile("msr ttbr0_el1, %0" : : "r"(pgd));
    __asm__ volatile("isb");
    __asm__ volatile("tlbi vmalle1");
    __asm__ volatile("dsb sy");
    __asm__ volatile("isb");
}

/* ── Kernel paging ───────────────────────────────────────────────── */

void arm64_paging_init(void) {
    printf("[PAGING] ARM64 4-level page tables\n");

    /* Read current TTBR1_EL1 */
    uintptr_t ttbr1;
    __asm__ volatile("mrs %0, ttbr1_el1" : "=r"(ttbr1));
    kernel_pgd = ttbr1;
    printf("[PAGING] Kernel TTBR1 = 0x%lx\n", kernel_pgd);
}

/* ── CPU init ────────────────────────────────────────────────────── */

void arm64_cpu_init(void) {
    printf("[CPU] ARM64 secondary CPU online\n");
}
