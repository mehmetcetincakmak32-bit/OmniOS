/* OmniOS — kernel/mm/vmm.h */
/* Virtual Memory Manager — regions, mmap, page fault handler */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_VMM_H
#define OMNIOS_VMM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define VM_REGION_MAX   1024
#define VM_STACK_PAGES  16

/* VM region flags */
#define VM_READ     (1 << 0)
#define VM_WRITE    (1 << 1)
#define VM_EXEC     (1 << 2)
#define VM_USER     (1 << 3)
#define VM_SHARED   (1 << 4)
#define VM_IO       (1 << 5)
#define VM_STACK    (1 << 6)
#define VM_HEAP     (1 << 7)
#define VM_MMIO     (1 << 8)

typedef struct {
    uintptr_t start;
    uintptr_t end;
    uint64_t  flags;
    uint32_t  id;
    uintptr_t phys_base;   /* For mmap'd physical memory */
    bool      used;
} vm_region_t;

typedef struct {
    vm_region_t regions[VM_REGION_MAX];
    uintptr_t   heap_start;
    uintptr_t   heap_end;
    uintptr_t   mmap_base;
    uint32_t    region_count;
    uintptr_t   pml4_phys;
} vm_space_t;

void vmm_init(void);
vm_space_t *vmm_create_space(void);
void vmm_destroy_space(vm_space_t *space);
void vmm_switch_space(vm_space_t *space);
int vmm_map_pages(vm_space_t *space, uintptr_t virt, uintptr_t phys,
    size_t pages, uint64_t flags);
void vmm_unmap_pages(vm_space_t *space, uintptr_t virt, size_t pages);
uintptr_t vmm_find_free_region(vm_space_t *space, size_t size);
int vmm_handle_page_fault(uintptr_t fault_addr, uint64_t error_code);
void vmm_print_regions(vm_space_t *space);

/* Kernel heap */
void *vmm_alloc_kernel_heap(size_t size);
void vmm_free_kernel_heap(void *ptr, size_t size);

#endif /* OMNIOS_VMM_H */
