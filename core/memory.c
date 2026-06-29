/*
 * OmniOS Memory Manager
 * Lightweight memory tracking and allocation
 */

#include "include/omnios_core.h"
#include <string.h>
#include <stdlib.h>

#define MAX_BLOCKS 128

typedef struct {
    uint32_t pid;
    uint32_t size_mb;
    bool     pinned;
    char     label[32];
} MemoryBlock;

static MemoryBlock _blocks[MAX_BLOCKS];
static uint32_t _block_count = 0;
static uint32_t _total_memory_mb = 4096;

void om_memory_init(uint32_t total_mb) {
    _total_memory_mb = total_mb;
    _block_count = 0;
    memset(_blocks, 0, sizeof(_blocks));
}

bool om_memory_allocate(uint32_t pid, uint32_t size_mb) {
    uint32_t available = om_memory_get_available();

    if (size_mb > available)
        return false;

    if (_block_count >= MAX_BLOCKS)
        return false;

    _blocks[_block_count].pid = pid;
    _blocks[_block_count].size_mb = size_mb;
    _blocks[_block_count].pinned = false;
    _block_count++;
    return true;
}

bool om_memory_free(uint32_t pid) {
    for (uint32_t i = 0; i < _block_count; i++) {
        if (_blocks[i].pid == pid) {
            _blocks[i] = _blocks[_block_count - 1];
            _block_count--;
            return true;
        }
    }
    return false;
}

uint32_t om_memory_get_used(void) {
    uint32_t used = 0;
    for (uint32_t i = 0; i < _block_count; i++) {
        if (!_blocks[i].pinned)
            used += _blocks[i].size_mb;
    }
    return used;
}

uint32_t om_memory_get_available(void) {
    uint32_t used = 0;
    for (uint32_t i = 0; i < _block_count; i++) {
        used += _blocks[i].size_mb;
    }
    return (used > _total_memory_mb) ? 0 : (_total_memory_mb - used);
}

float om_memory_get_usage_percent(void) {
    uint32_t used = _total_memory_mb - om_memory_get_available();
    return (float)used / (float)_total_memory_mb * 100.0f;
}
