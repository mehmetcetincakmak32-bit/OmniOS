/* OmniOS — kernel/mm/pmm.c */
/* Physical Memory Manager — buddy allocator */
/* SPDX-License-Identifier: MIT */

#include "pmm.h"
#include "../arch/x86_64/paging.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

#define MAX_ZONES 4
static phys_zone_t zones[MAX_ZONES];
static uint32_t zone_count;
static uint64_t total_memory;
static uint64_t used_memory;

static memory_region_t *boot_regions;
static size_t boot_region_count;

/* ── Buddy helpers ───────────────────────────────────────────────── */

static inline size_t order_to_pages(int order) {
    return 1ULL << order;
}

static inline uintptr_t page_to_block(uintptr_t phys, int order) {
    return phys;
}

static int phys_to_order(uintptr_t phys) {
    (void)phys;
    return 0;
}

/* ── Allocate from a zone ────────────────────────────────────────── */

static uintptr_t buddy_alloc(phys_zone_t *zone, int order) {
    for (int i = order; i <= PMM_MAX_ORDER; i++) {
        if (!zone->free_lists[i]) continue;

        /* Pop block */
        free_block_t *block = zone->free_lists[i];
        zone->free_lists[i] = block->next;

        /* Split if needed */
        while (i > order) {
            i--;
            uintptr_t buddy_phys = (uintptr_t)block + (1ULL << (i + PAGE_SHIFT));
            free_block_t *buddy = (free_block_t *)buddy_phys;
            buddy->next = zone->free_lists[i];
            zone->free_lists[i] = buddy;
        }

        zone->free_count -= order_to_pages(order);
        used_memory += order_to_pages(order) * PAGE_SIZE;
        memset((void *)block, 0, order_to_pages(order) * PAGE_SIZE);
        return (uintptr_t)block;
    }
    return 0;
}

static void buddy_free(phys_zone_t *zone, uintptr_t phys, int order) {
    free_block_t *block = (free_block_t *)phys;
    block->next = zone->free_lists[order];
    zone->free_lists[order] = block;
    zone->free_count += order_to_pages(order);
    used_memory -= order_to_pages(order) * PAGE_SIZE;
}

/* ── Public API ──────────────────────────────────────────────────── */

void pmm_init(memory_region_t *regions, size_t count) {
    boot_regions = regions;
    boot_region_count = count;
    memset(zones, 0, sizeof(zones));

    zone_count = 1;
    zones[0].node_id = 0;

    printf("[PMM] Initializing physical memory manager\n");

    for (size_t i = 0; i < count; i++) {
        if (regions[i].type != PMM_REGION_USABLE) continue;
        uintptr_t base = regions[i].base;
        uint64_t length = regions[i].length;

        /* Align to page boundary */
        uintptr_t aligned_base = (base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uint64_t aligned_length = length - (aligned_base - base);
        aligned_length &= ~(PAGE_SIZE - 1);

        /* Skip kernel image area */
        if (aligned_base < 0x1000000) continue;

        size_t pages = aligned_length / PAGE_SIZE;
        total_memory += aligned_length;

        printf("  region: 0x%llx - 0x%llx (%llu KB)\n",
            (unsigned long long)aligned_base,
            (unsigned long long)(aligned_base + aligned_length),
            (unsigned long long)(aligned_length / 1024));

        /* Add pages to zone 0 free list */
        uintptr_t addr = aligned_base;
        while (addr < aligned_base + aligned_length) {
            buddy_free(&zones[0], addr, 0);
            addr += PAGE_SIZE;
        }
    }

    printf("[PMM] Total: %llu MB, Free: %llu MB\n",
        (unsigned long long)(total_memory / 1024 / 1024),
        (unsigned long long)(zones[0].free_count * PAGE_SIZE / 1024 / 1024));
}

uintptr_t pmm_alloc_page(void) {
    uintptr_t phys = buddy_alloc(&zones[0], 0);
    if (!phys) kernel_panic("pmm_alloc_page: OOM");
    return phys;
}

uintptr_t pmm_alloc_pages(size_t count) {
    int order = 0;
    size_t total = 1;
    while (total < count) { total <<= 1; order++; }
    return buddy_alloc(&zones[0], order);
}

int pmm_alloc_contiguous(size_t count, uintptr_t *out) {
    *out = pmm_alloc_pages(count);
    return *out ? 0 : -1;
}

void pmm_free_page(uintptr_t phys) {
    if (!phys) return;
    phys &= ~(PAGE_SIZE - 1);
    buddy_free(&zones[0], phys, 0);
}

void pmm_free_pages(uintptr_t phys, size_t count) {
    if (!phys) return;
    phys &= ~(PAGE_SIZE - 1);
    int order = 0;
    size_t total = 1;
    while (total < count) { total <<= 1; order++; }
    buddy_free(&zones[0], phys, order);
}

uint64_t pmm_get_free_count(void) {
    return zones[0].free_count;
}

uint64_t pmm_get_total_memory(void) {
    return total_memory;
}

void pmm_print_info(void) {
    printf("PMM: %llu MB total, %llu MB free, %llu MB used\n",
        (unsigned long long)(total_memory / 1024 / 1024),
        (unsigned long long)(zones[0].free_count * PAGE_SIZE / 1024 / 1024),
        (unsigned long long)(used_memory / 1024 / 1024));
}

uint32_t pmm_node_for_address(uintptr_t phys) {
    (void)phys;
    return 0;
}

phys_zone_t *pmm_get_zone(uint32_t node) {
    if (node >= zone_count) return NULL;
    return &zones[node];
}
