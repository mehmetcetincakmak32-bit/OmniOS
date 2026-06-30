/* OmniOS — kernel/mm/vmm.c */
/* Virtual Memory Manager */
/* SPDX-License-Identifier: MIT */

#include "vmm.h"
#include "pmm.h"
#include "../arch/x86_64/paging.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

static vm_space_t kernel_space;
static vm_space_t *current_space;

static uint32_t next_region_id = 1;

void vmm_init(void) {
    memset(&kernel_space, 0, sizeof(kernel_space));
    kernel_space.pml4_phys = paging_current_pml4();
    kernel_space.heap_start = KERNEL_HEAP_BASE;
    kernel_space.heap_end = KERNEL_HEAP_BASE;
    current_space = &kernel_space;
    printf("[VMM] Kernel virtual memory space initialized\n");
}

vm_space_t *vmm_create_space(void) {
    vm_space_t *space = (vm_space_t *)pmm_alloc_page();
    if (!space) return NULL;
    memset(space, 0, sizeof(*space));

    /* Allocate PML4 page */
    uintptr_t pml4_phys = pmm_alloc_page();
    if (!pml4_phys) { pmm_free_page((uintptr_t)space); return NULL; }

    /* Clone kernel higher half from current space */
    page_table_t *kernel_pml4 = (page_table_t *)KERNEL_VMA_BASE;
    page_table_t *new_pml4 = (page_table_t *)(pml4_phys + KERNEL_VMA_BASE);
    memset(new_pml4, 0, PAGE_SIZE);
    new_pml4->entries[KERNEL_PML4_IDX] = kernel_pml4->entries[KERNEL_PML4_IDX];

    space->pml4_phys = pml4_phys;
    space->heap_start = 0x6000000000ULL;
    space->heap_end = space->heap_start;
    space->mmap_base = 0x7000000000ULL;

    return space;
}

void vmm_destroy_space(vm_space_t *space) {
    if (!space || space == &kernel_space) return;
    for (uint32_t i = 0; i < VM_REGION_MAX; i++) {
        if (!space->regions[i].used) continue;
        vmm_unmap_pages(space, space->regions[i].start,
            (space->regions[i].end - space->regions[i].start) / PAGE_SIZE);
    }
    pmm_free_page(space->pml4_phys);
    pmm_free_page((uintptr_t)space);
}

void vmm_switch_space(vm_space_t *space) {
    if (!space) return;
    current_space = space;
    paging_switch(space->pml4_phys);
}

int vmm_map_pages(vm_space_t *space, uintptr_t virt, uintptr_t phys,
        size_t pages, uint64_t flags) {
    uint64_t pte_flags = PTE_PRESENT;
    if (flags & VM_WRITE) pte_flags |= PTE_WRITABLE;
    if (flags & VM_USER)  pte_flags |= PTE_USER;
    if (!(flags & VM_EXEC)) pte_flags |= PTE_NX;

    /* If mapping to current space, map now; otherwise just record */
    if (space == current_space || space == &kernel_space) {
        for (size_t i = 0; i < pages; i++) {
            paging_map_page(virt + i * PAGE_SIZE,
                phys ? phys + i * PAGE_SIZE : pmm_alloc_page(),
                pte_flags);
        }
    }

    /* Record region */
    for (uint32_t i = 0; i < VM_REGION_MAX; i++) {
        if (space->regions[i].used) continue;
        space->regions[i].start = virt;
        space->regions[i].end = virt + pages * PAGE_SIZE;
        space->regions[i].flags = flags;
        space->regions[i].id = next_region_id++;
        space->regions[i].phys_base = phys;
        space->regions[i].used = true;
        space->region_count++;
        break;
    }
    return 0;
}

void vmm_unmap_pages(vm_space_t *space, uintptr_t virt, size_t pages) {
    if (space == current_space || space == &kernel_space) {
        for (size_t i = 0; i < pages; i++) {
            paging_unmap_page(virt + i * PAGE_SIZE);
        }
    }
    /* Clear region */
    for (uint32_t i = 0; i < VM_REGION_MAX; i++) {
        if (space->regions[i].start == virt) {
            space->regions[i].used = false;
            space->region_count--;
            break;
        }
    }
}

uintptr_t vmm_find_free_region(vm_space_t *space, size_t size) {
    uintptr_t addr = space->heap_end;
    if (addr < USER_BASE) addr = USER_BASE;
    return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

int vmm_handle_page_fault(uintptr_t fault_addr, uint64_t error_code) {
    bool present = error_code & PF_PRESENT;
    bool write   = error_code & PF_WRITE;
    bool user    = error_code & PF_USER;

    if (!present && !user) {
        /* Lazy allocation: demand page for kernel */
        uintptr_t phys = pmm_alloc_page();
        paging_map_page(fault_addr & ~0xFFF, phys,
            PTE_PRESENT | PTE_WRITABLE | PTE_NX);
        return 0;
    }

    printf("[VMM] Page fault: addr=0x%llx error=%llx %s%s%s\n",
        (unsigned long long)fault_addr, (unsigned long long)error_code,
        present ? "PRESENT " : "", write ? "WRITE " : "", user ? "USER" : "");
    return -1;
}

void vmm_print_regions(vm_space_t *space) {
    printf("VMM regions (%u):\n", space->region_count);
    for (uint32_t i = 0; i < VM_REGION_MAX; i++) {
        if (!space->regions[i].used) continue;
        printf("  [%u] 0x%llx-0x%llx flags=%llx\n",
            space->regions[i].id,
            (unsigned long long)space->regions[i].start,
            (unsigned long long)space->regions[i].end,
            (unsigned long long)space->regions[i].flags);
    }
}

void *vmm_alloc_kernel_heap(size_t size) {
    size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uintptr_t addr = kernel_space.heap_end;
    vmm_map_pages(&kernel_space, addr, 0, size / PAGE_SIZE, VM_READ | VM_WRITE);
    kernel_space.heap_end += size;
    return (void *)addr;
}

void vmm_free_kernel_heap(void *ptr, size_t size) {
    (void)ptr;
    (void)size;
}
