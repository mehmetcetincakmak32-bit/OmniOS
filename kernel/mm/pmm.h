/* OmniOS — kernel/mm/pmm.h */
/* Physical Memory Manager — buddy allocator, NUMA-aware */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_PMM_H
#define OMNIOS_PMM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define PMM_MAX_ORDER    10     /* 2^10 * 4K = 4MB max block */
#define PMM_NUM_ORDERS   (PMM_MAX_ORDER + 1)
#define PMM_BITMAP_SIZE  0x100000  /* ~512MB of bitmap coverage */

/* Memory region types (from bootloader / e820) */
#define PMM_REGION_USABLE       1
#define PMM_REGION_RESERVED     2
#define PMM_REGION_ACPI_RECLAIM 3
#define PMM_REGION_ACPI_NVS     4
#define PMM_REGION_BAD          5

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
} __attribute__((packed)) memory_region_t;

/* ── Per-zone free list (NUMA node) ──────────────────────────────── */

typedef struct free_block {
    struct free_block *next;
} free_block_t;

typedef struct {
    free_block_t *free_lists[PMM_NUM_ORDERS];
    uint64_t      free_count;
    uint64_t      total_pages;
    uint32_t      node_id;
} phys_zone_t;

/* ── Public API ──────────────────────────────────────────────────── */

void pmm_init(memory_region_t *regions, size_t count);
uintptr_t pmm_alloc_page(void);
uintptr_t pmm_alloc_pages(size_t count);
int pmm_alloc_contiguous(size_t count, uintptr_t *out);
void pmm_free_page(uintptr_t phys);
void pmm_free_pages(uintptr_t phys, size_t count);
uint64_t pmm_get_free_count(void);
uint64_t pmm_get_total_memory(void);
void pmm_print_info(void);

/* NUMA awareness */
uint32_t pmm_node_for_address(uintptr_t phys);
phys_zone_t *pmm_get_zone(uint32_t node);

#endif /* OMNIOS_PMM_H */
