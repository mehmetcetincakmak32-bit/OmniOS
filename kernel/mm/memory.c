/*
 * OmniOS Memory Manager
 * Physical page allocator, virtual memory, heap
 */

#include "../include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

#define HEAP_SIZE           (64 * 1024 * 1024)  /* 64 MB heap */
#define HEAP_MAGIC          0xDEADBEEF
#define MIN_BLOCK_SIZE      32
#define ALIGNMENT           8

/* Physical memory region */
typedef struct {
    uintptr_t base;
    uintptr_t size;
    bool      free;
} mem_region_t;

/* Heap block header */
typedef struct heap_block {
    uint32_t        magic;
    size_t          size;
    bool            free;
    struct heap_block* next;
    struct heap_block* prev;
    /* data follows immediately */
} heap_block_t;

/* Physical memory state */
static mem_region_t _regions[32];
static uint32_t _region_count = 0;
static uint32_t _total_memory = 0;
static uint32_t _used_memory = 0;

/* Heap state */
static uint8_t* _heap_start = NULL;
static size_t _heap_size = 0;
static heap_block_t* _heap_first = NULL;
static uint32_t _heap_allocs = 0;
static uint32_t _heap_frees = 0;

/* Page allocator bitmap */
#define MAX_PAGES (4096 * 1024 / PAGE_SIZE)  /* For 4GB RAM */
static uint8_t _page_bitmap[MAX_PAGES / 8];
static uint32_t _total_pages = 0;
static uint32_t _free_pages = 0;

/* ================================================================
 * Physical Page Allocator
 * ================================================================ */

status_t mm_init(uint32_t total_memory_mb) {
    _total_memory = total_memory_mb;
    _used_memory = 0;

    /* Calculate total pages */
    _total_pages = (total_memory_mb * 1024 * 1024) / PAGE_SIZE;
    _free_pages = _total_pages;

    if (_total_pages > MAX_PAGES) {
        _total_pages = MAX_PAGES;
        _free_pages = _total_pages;
    }

    /* Clear bitmap */
    memset(_page_bitmap, 0, sizeof(_page_bitmap));

    /* Mark first 16MB as used (kernel + reserved) */
    uint32_t reserved_pages = (16 * 1024 * 1024) / PAGE_SIZE;
    for (uint32_t i = 0; i < reserved_pages && i < _total_pages; i++) {
        _page_bitmap[i / 8] |= (1 << (i % 8));
        _free_pages--;
        _used_memory += PAGE_SIZE;
    }

    /* Initialize heap */
    _heap_size = HEAP_SIZE;
    _heap_start = (uint8_t*)kmalloc(HEAP_SIZE);
    if (!_heap_start) {
        /* Fallback: use static buffer */
        static uint8_t _heap_buffer[HEAP_SIZE];
        _heap_start = _heap_buffer;
    }

    /* Initialize first heap block */
    _heap_first = (heap_block_t*)_heap_start;
    _heap_first->magic = HEAP_MAGIC;
    _heap_first->size = _heap_size - sizeof(heap_block_t);
    _heap_first->free = true;
    _heap_first->next = NULL;
    _heap_first->prev = NULL;

    _heap_allocs = 0;
    _heap_frees = 0;

    printf("[MM] %d MB toplam bellek\n", total_memory_mb);
    printf("[MM] %d sayfa (%d MB kullanilabilir)\n",
           _total_pages, (_total_pages * PAGE_SIZE) / (1024 * 1024));
    printf("[MM] %d MB yigin baslatildi\n", HEAP_SIZE / (1024 * 1024));

    return STATUS_SUCCESS;
}

page_t* mm_alloc_page(page_flags_t flags) {
    if (_free_pages == 0) return NULL;

    /* Find free page */
    for (uint32_t i = 0; i < _total_pages; i++) {
        if (!(_page_bitmap[i / 8] & (1 << (i % 8)))) {
            /* Mark as used */
            _page_bitmap[i / 8] |= (1 << (i % 8));
            _free_pages--;
            _used_memory += PAGE_SIZE;

            /* Allocate page struct */
            page_t* page = (page_t*)kmalloc(sizeof(page_t));
            if (!page) return NULL;

            page->phys_addr = i * PAGE_SIZE;
            page->virt_addr = 0;
            page->flags = flags;
            page->ref_count = 1;
            page->next = NULL;

            return page;
        }
    }
    return NULL;
}

status_t mm_free_page(page_t* page) {
    if (!page) return STATUS_INVALID;

    if (page->ref_count > 1) {
        page->ref_count--;
        return STATUS_SUCCESS;
    }

    uint32_t page_idx = page->phys_addr / PAGE_SIZE;
    if (page_idx < _total_pages) {
        _page_bitmap[page_idx / 8] &= ~(1 << (page_idx % 8));
        _free_pages++;
        _used_memory -= PAGE_SIZE;
    }

    kfree(page);
    return STATUS_SUCCESS;
}

uint32_t mm_get_total_pages(void) { return _total_pages; }
uint32_t mm_get_free_pages(void) { return _free_pages; }
uint32_t mm_get_used_memory(void) { return _used_memory / (1024 * 1024); }

/* ================================================================
 * Heap Allocator (kmalloc/kfree)
 * ================================================================ */

void* kmalloc(size_t size) {
    if (size == 0) return NULL;
    if (!_heap_first) return NULL;

    /* Align to MIN_BLOCK_SIZE */
    size = (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    if (size < MIN_BLOCK_SIZE) size = MIN_BLOCK_SIZE;

    /* Find free block */
    heap_block_t* block = _heap_first;
    while (block) {
        if (block->free && block->size >= size) {
            /* Split block if too large */
            if (block->size > size + sizeof(heap_block_t) + MIN_BLOCK_SIZE) {
                heap_block_t* new_block = (heap_block_t*)((uint8_t*)block + sizeof(heap_block_t) + size);
                new_block->magic = HEAP_MAGIC;
                new_block->size = block->size - size - sizeof(heap_block_t);
                new_block->free = true;
                new_block->next = block->next;
                new_block->prev = block;

                if (block->next)
                    block->next->prev = new_block;
                block->next = new_block;
                block->size = size;
            }

            block->free = false;
            block->magic = HEAP_MAGIC;
            _heap_allocs++;
            return (void*)((uint8_t*)block + sizeof(heap_block_t));
        }
        block = block->next;
    }

    /* No suitable block found */
    return NULL;
}

void* kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = kmalloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) { kfree(ptr); return NULL; }

    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) return NULL;

    size_t old_size = block->size;
    void* new_ptr = kmalloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size < size ? old_size : size);
        kfree(ptr);
    }
    return new_ptr;
}

void kfree(void* ptr) {
    if (!ptr) return;

    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) return;

    block->free = true;
    _heap_frees++;

    /* Coalesce with next block */
    if (block->next && block->next->free) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
    }

    /* Coalesce with prev block */
    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(heap_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next)
            block->next->prev = block->prev;
    }
}

/* I/O memory mapping stub */
void* mm_map_io(uintptr_t phys_addr, size_t size) {
    /* In real implementation: set up page table entries for MMIO */
    (void)phys_addr;
    (void)size;
    return NULL;
}
