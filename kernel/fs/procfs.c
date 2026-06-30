/* OmniOS — kernel/fs/procfs.c */
/* Process filesystem — /proc entries */
/* SPDX-License-Identifier: MIT */

#include "../include/omnios_kernel.h"
#include "../proc/process.h"
#include "../sched/sched.h"
#include <stdio.h>
#include <string.h>

#define PROCFS_MAX_ENTRIES  32

typedef struct {
    char  name[32];
    int   (*read)(char *buf, size_t size);
} procfs_entry_t;

static procfs_entry_t _procfs_entries[PROCFS_MAX_ENTRIES];
static int _procfs_count = 0;
static int _procfs_initialized = 0;

static int proc_read_cpuinfo(char *buf, size_t size) {
    return snprintf(buf, size,
        "processor\t: 0\n"
        "model name\t: OmniOS ARM64 Mobile\n"
        "BogoMIPS\t: 100.00\n"
        "Features\t: fp asimd evtstrm aes pmull sha1 sha2 crc32\n"
        "CPU implementer\t: 0x51\n"
        "CPU architecture: 8\n"
        "CPU variant\t: 0x0\n"
        "CPU part\t: 0x003\n"
        "CPU revision\t: 0\n");
}

static int proc_read_meminfo(char *buf, size_t size) {
    uint64_t total_kb = pmm_get_total_memory() / 1024;
    uint64_t free_kb  = pmm_get_free_count() * 4;  /* Convert pages to KB */
    return snprintf(buf, size,
        "MemTotal:      %8llu kB\n"
        "MemFree:       %8llu kB\n"
        "MemAvailable:  %8llu kB\n",
        (unsigned long long)total_kb,
        (unsigned long long)free_kb,
        (unsigned long long)free_kb);
}

static int proc_read_uptime(char *buf, size_t size) {
    time_t ms = kernel_get_uptime_ms();
    return snprintf(buf, size, "%llu.%02u\n",
        (unsigned long long)(ms / 1000),
        (unsigned int)(ms % 1000) / 10);
}

static int proc_read_version(char *buf, size_t size) {
    return snprintf(buf, size,
        "OmniOS version 1.0 (ARM64 Mobile)\n"
        "#1 SMP Tue Jun 30 2026\n");
}

static int proc_read_battery(char *buf, size_t size) {
#ifdef CONFIG_ARM64
    extern int pm8150_battery_soc(void);
    extern int pm8150_battery_voltage(void);
    extern int pm8150_charger_status(void);
    return snprintf(buf, size,
        "capacity: %d\nvoltage: %d\nstatus: %s\n",
        pm8150_battery_soc(), pm8150_battery_voltage(),
        pm8150_charger_status() ? "Charging" : "Discharging");
#else
    return snprintf(buf, size, "capacity: 0\n");
#endif
}

void procfs_init(void) {
    memset(_procfs_entries, 0, sizeof(_procfs_entries));
    _procfs_count = 0;
    _procfs_initialized = 1;

    procfs_add("cpuinfo",  proc_read_cpuinfo);
    procfs_add("meminfo",  proc_read_meminfo);
    procfs_add("uptime",   proc_read_uptime);
    procfs_add("version",  proc_read_version);
    procfs_add("battery",  proc_read_battery);

    printf("[PROCFS] /proc initialized (%d entries)\n", _procfs_count);
}

int procfs_add(const char *name, int (*read_fn)(char *, size_t)) {
    if (!_procfs_initialized || _procfs_count >= PROCFS_MAX_ENTRIES) return -1;
    strncpy(_procfs_entries[_procfs_count].name, name, 31);
    _procfs_entries[_procfs_count].name[31] = '\0';
    _procfs_entries[_procfs_count].read = read_fn;
    _procfs_count++;
    return 0;
}

int procfs_read(const char *name, char *buf, size_t size) {
    for (int i = 0; i < _procfs_count; i++) {
        if (strcmp(_procfs_entries[i].name, name) == 0) {
            if (_procfs_entries[i].read)
                return _procfs_entries[i].read(buf, size);
            return 0;
        }
    }
    return -1;
}

void procfs_list(void) {
    printf("[PROCFS] /proc entries:\n");
    for (int i = 0; i < _procfs_count; i++) {
        printf("  %s\n", _procfs_entries[i].name);
    }
}
