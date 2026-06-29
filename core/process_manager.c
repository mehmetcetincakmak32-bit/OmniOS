/*
 * OmniOS Process Manager
 * Native process lifecycle management
 */

#include "include/omnios_core.h"
#include <string.h>
#include <stdlib.h>

#define MAX_PROCESSES 64

static Process _process_table[MAX_PROCESSES];
static uint32_t _process_count = 0;
static uint32_t _next_pid = 1;
static uint32_t _total_created = 0;

static int _find_slot(void) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (_process_table[i].state == PROC_TERMINATED ||
            _process_table[i].state == PROC_CRASHED) {
            memset(&_process_table[i], 0, sizeof(Process));
            return i;
        }
    }
    if (_process_count < MAX_PROCESSES)
        return _process_count;
    return -1;
}

static int _find_by_pid(uint32_t pid) {
    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        if (_process_table[i].pid == pid &&
            _process_table[i].state != PROC_TERMINATED &&
            _process_table[i].state != PROC_CRASHED)
            return i;
    }
    return -1;
}

Process* om_process_create(const char* name, PlatformType platform) {
    int slot = _find_slot();
    if (slot < 0) return NULL;

    Process* proc = &_process_table[slot];
    proc->pid = _next_pid++;
    strncpy(proc->name, name, sizeof(proc->name) - 1);
    proc->platform = platform;
    proc->state = PROC_RUNNING;
    proc->cpu_usage = 0.0f;
    proc->mem_usage = 0.0f;
    proc->priority = 0;
    proc->created_at = _total_created;

    _process_count++;
    _total_created++;
    return proc;
}

bool om_process_kill(uint32_t pid) {
    int idx = _find_by_pid(pid);
    if (idx < 0) return false;

    _process_table[idx].state = PROC_TERMINATED;
    _process_count--;
    return true;
}

bool om_process_suspend(uint32_t pid) {
    int idx = _find_by_pid(pid);
    if (idx < 0) return false;

    if (_process_table[idx].state != PROC_RUNNING)
        return false;

    _process_table[idx].state = PROC_SUSPENDED;
    return true;
}

bool om_process_resume(uint32_t pid) {
    int idx = _find_by_pid(pid);
    if (idx < 0) return false;

    if (_process_table[idx].state != PROC_SUSPENDED)
        return false;

    _process_table[idx].state = PROC_RUNNING;
    return true;
}

Process* om_process_get(uint32_t pid) {
    int idx = _find_by_pid(pid);
    if (idx < 0) return NULL;
    return &_process_table[idx];
}

uint32_t om_process_count(void) {
    return _process_count;
}

void om_process_list(Process* buffer, uint32_t* count) {
    if (!buffer || !count) return;

    uint32_t written = 0;
    uint32_t max = *count;

    for (uint32_t i = 0; i < MAX_PROCESSES && written < max; i++) {
        if (_process_table[i].state == PROC_RUNNING ||
            _process_table[i].state == PROC_BACKGROUND) {
            memcpy(&buffer[written], &_process_table[i], sizeof(Process));
            written++;
        }
    }
    *count = written;
}
