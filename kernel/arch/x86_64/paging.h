/* OmniOS — kernel/arch/x86_64/paging.h */
/* 4-level paging (5-level capable), PCID, NX, SMEP/SMAP */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_PAGING_H
#define OMNIOS_PAGING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ── Page table entry flags (x86_64) ─────────────────────────────── */

#define PTE_PRESENT      (1ULL << 0)
#define PTE_WRITABLE     (1ULL << 1)
#define PTE_USER         (1ULL << 2)
#define PTE_WRITETHROUGH (1ULL << 3)
#define PTE_CACHE_DISABLE (1ULL << 4)
#define PTE_ACCESSED     (1ULL << 5)
#define PTE_DIRTY        (1ULL << 6)
#define PTE_HUGE         (1ULL << 7)   /* 2MB/1GB page */
#define PTE_GLOBAL       (1ULL << 8)
#define PTE_NX           (1ULL << 63)

/* Page sizes */
#define PAGE_SIZE       4096
#define PAGE_SHIFT      12
#define PAGE_MASK       (~0xFFFULL)
#define HUGE_PAGE_SIZE  (2 * 1024 * 1024)
#define HUGE_PAGE_SHIFT 21

/* ── Higher-half memory layout ───────────────────────────────────── */

#define KERNEL_VMA_BASE  0xFFFFFFFF80000000ULL
#define KERNEL_PML4_IDX  511   /* Canonical higher-half entry */
#define KERNEL_HEAP_BASE 0xFFFFFFFF90000000ULL
#define KERNEL_HEAP_SIZE (256ULL * 1024 * 1024)
#define KERNEL_STACK_SIZE 16384
#define USER_BASE        0x0000000000400000ULL
#define USER_STACK_TOP   0x00007FFFFFFFFFFFULL

/* ── Page table entry types ──────────────────────────────────────── */

typedef uint64_t pte_t;

typedef struct {
    pte_t entries[512];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

typedef struct {
    uint64_t pml4;    /* PML4 physical address */
    uint64_t pdp;     /* PDP physical address */
    uint64_t pd;      /* PD physical address */
    uint64_t pt;      /* PT physical address */
    uintptr_t pdpt_higher;  /* PDP for higher half */
    uintptr_t pd_higher;    /* PD for higher half */
} page_map_t;

/* ── Page fault error codes ──────────────────────────────────────── */

#define PF_PRESENT  (1 << 0)
#define PF_WRITE    (1 << 1)
#define PF_USER     (1 << 2)
#define PF_RSVD     (1 << 3)
#define PF_INSTR    (1 << 4)
#define PF_PKEY     (1 << 5)

/* ── Public API ──────────────────────────────────────────────────── */

void paging_init(void);
void paging_map_page(uintptr_t virt, uintptr_t phys, uint64_t flags);
void paging_unmap_page(uintptr_t virt);
uintptr_t paging_virt_to_phys(uintptr_t virt);
void paging_switch(uintptr_t pml4_phys);
uintptr_t paging_current_pml4(void);

/* PCID helpers */
void paging_invalidate_tlb(void);
void paging_invalidate_tlb_entry(uintptr_t virt);
void paging_enable_pcid(void);

/* Region-based mapping */
int paging_map_region(uintptr_t virt, uintptr_t phys, size_t size, uint64_t flags);
void paging_unmap_region(uintptr_t virt, size_t size);

/* Identity map for early boot */
void paging_early_map(uintptr_t phys, size_t size);

#endif /* OMNIOS_PAGING_H */
