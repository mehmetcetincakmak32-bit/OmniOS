/* OmniOS — kernel/arch/x86_64/paging.c */
/* 4-level paging for x86_64 with PCID, NX, SMEP/SMAP */
/* SPDX-License-Identifier: MIT */

#include "paging.h"
#include "io.h"
#include "../../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Page table helpers ──────────────────────────────────────────── */

static inline uint64_t get_cr3(void) {
    uint64_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void set_cr3(uint64_t cr3) {
    __asm__ volatile("mov %0, %%cr3; mov %%cr3, %%r12" : : "r"(cr3) : "r12");
}

static inline uint64_t read_cr0(void) {
    uint64_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    return cr0;
}

static inline void write_cr4(uint64_t val) {
    __asm__ volatile("mov %0, %%cr4" : : "r"(val) : "memory");
}

static inline uint64_t read_cr4(void) {
    uint64_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

/* Temporary page tables (defined in boot.S) */
extern uint64_t pml4[512];
extern uint64_t pdp[512];
extern uint64_t pd[512];
extern uint64_t pt[512];

static uint64_t kernel_pml4_phys;
static bool pcid_enabled = false;

/* ── PCID management ─────────────────────────────────────────────── */

void paging_enable_pcid(void) {
    uint64_t cr4 = read_cr4();
    cr4 |= (1 << 17);  /* PCIDE */
    write_cr4(cr4);
    pcid_enabled = true;
}

void paging_invalidate_tlb(void) {
    if (pcid_enabled) {
        /* INVPCID with global flag */
        struct { uint64_t addr; uint64_t pcid; } desc = {0, 0};
        __asm__ volatile("invpcid %0, %1" : : "m"(desc), "r"(0ULL) : "memory");
    } else {
        set_cr3(get_cr3());
    }
}

void paging_invalidate_tlb_entry(uintptr_t virt) {
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

/* ── Page table walk ─────────────────────────────────────────────── */

static void *phys_to_virt(uintptr_t phys) {
    if (phys >= KERNEL_VMA_BASE) return (void *)phys;
    return (void *)(phys + KERNEL_VMA_BASE);
}

static pte_t *walk(uintptr_t virt, bool create) {
    int pml4_idx = (virt >> 39) & 0x1FF;
    int pdp_idx  = (virt >> 30) & 0x1FF;
    int pd_idx   = (virt >> 21) & 0x1FF;
    int pt_idx   = (virt >> 12) & 0x1FF;

    page_table_t *pml4_table = phys_to_virt(kernel_pml4_phys);

    /* PML4 entry */
    pte_t *pml4e = &pml4_table->entries[pml4_idx];
    if (!(*pml4e & PTE_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new_page = (uintptr_t)aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        memset((void *)new_page, 0, PAGE_SIZE);
        *pml4e = (new_page & PAGE_MASK) | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    }

    page_table_t *pdp_table = phys_to_virt(*pml4e & PAGE_MASK);
    pte_t *pdpe = &pdp_table->entries[pdp_idx];
    if (!(*pdpe & PTE_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new_page = (uintptr_t)aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        memset((void *)new_page, 0, PAGE_SIZE);
        *pdpe = (new_page & PAGE_MASK) | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    }

    page_table_t *pd_table = phys_to_virt(*pdpe & PAGE_MASK);
    pte_t *pde = &pd_table->entries[pd_idx];
    if (!(*pde & PTE_PRESENT)) {
        if (!create) return NULL;
        uintptr_t new_page = (uintptr_t)aligned_alloc(PAGE_SIZE, PAGE_SIZE);
        memset((void *)new_page, 0, PAGE_SIZE);
        *pde = (new_page & PAGE_MASK) | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
    }

    page_table_t *pt_table = phys_to_virt(*pde & PAGE_MASK);
    return &pt_table->entries[pt_idx];
}

/* ── Page mapping operations ─────────────────────────────────────── */

void paging_map_page(uintptr_t virt, uintptr_t phys, uint64_t flags) {
    pte_t *entry = walk(virt, true);
    if (!entry) { kernel_panic("paging_map_page: walk failed"); return; }

    uint64_t pte = (phys & PAGE_MASK) | PTE_PRESENT | flags;
    if (phys < 0x100000000ULL) pte |= PTE_GLOBAL;
    *entry = pte;
    paging_invalidate_tlb_entry(virt);
}

void paging_unmap_page(uintptr_t virt) {
    pte_t *entry = walk(virt, false);
    if (!entry || !(*entry & PTE_PRESENT)) return;
    *entry = 0;
    paging_invalidate_tlb_entry(virt);
}

uintptr_t paging_virt_to_phys(uintptr_t virt) {
    pte_t *entry = walk(virt, false);
    if (!entry || !(*entry & PTE_PRESENT)) return 0;
    return (*entry & PAGE_MASK) | (virt & 0xFFF);
}

int paging_map_region(uintptr_t virt, uintptr_t phys, size_t size, uint64_t flags) {
    for (size_t offset = 0; offset < size; offset += PAGE_SIZE) {
        paging_map_page(virt + offset, phys + offset, flags);
    }
    return 0;
}

void paging_unmap_region(uintptr_t virt, size_t size) {
    for (size_t offset = 0; offset < size; offset += PAGE_SIZE) {
        paging_unmap_page(virt + offset);
    }
}

void paging_switch(uintptr_t pml4_phys) {
    set_cr3(pml4_phys);
}

uintptr_t paging_current_pml4(void) {
    return get_cr3();
}

/* ── Early identity map for transition ───────────────────────────── */

static bool early_mapped = false;

void paging_early_map(uintptr_t phys, size_t size) {
    if (!early_mapped) {
        /* Already have 1:1 mapping from boot.S */
        early_mapped = true;
    }
    paging_map_region(phys, phys, size, PTE_WRITABLE);
}

/* ── Initialize paging system ────────────────────────────────────── */

void paging_init(void) {
    /* Get PML4 phys from boot.S */
    kernel_pml4_phys = get_cr3() & PAGE_MASK;

    /* Fix up page tables: create higher-half mapping from current 1:1 */
    page_table_t *pml4_table = phys_to_virt(kernel_pml4_phys);
    pml4_table->entries[KERNEL_PML4_IDX] = kernel_pml4_phys | PTE_PRESENT | PTE_WRITABLE;

    /* Enable NX via EFER */
    uint32_t efer_lo, efer_hi;
    __asm__ volatile("rdmsr" : "=a"(efer_lo), "=d"(efer_hi) : "c"(0xC0000080));
    efer_lo |= (1 << 11);  /* NXE */
    __asm__ volatile("wrmsr" : : "a"(efer_lo), "d"(efer_hi), "c"(0xC0000080));

    /* Enable SMEP + SMAP + PCID */
    uint64_t cr4 = read_cr4();
    cr4 |= (1 << 20);  /* SMEP */
    cr4 |= (1 << 21);  /* SMAP */
    write_cr4(cr4);

    paging_enable_pcid();
    paging_invalidate_tlb();

    page_table_t *check = phys_to_virt(kernel_pml4_phys);
    printf("[PAGING] PML4 @ phys 0x%llx, higher-half @ entry %d\n",
        (unsigned long long)kernel_pml4_phys, KERNEL_PML4_IDX);
    printf("[PAGING] NX=%d SMEP=%d SMAP=%d PCID=%d\n",
        1, 1, 1, pcid_enabled);
}
