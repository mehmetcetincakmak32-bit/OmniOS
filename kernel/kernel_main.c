/*
 * OmniOS x86_64 Kernel — Main Entry Point
 * Full SMP + ACPI + PCI + VirtIO initialization
 */

#include "include/omnios_kernel.h"
#include "arch/x86_64/cpu.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/apic.h"
#include "arch/x86_64/paging.h"
#include "arch/x86_64/smp.h"
#include "arch/x86_64/interrupts.h"
#include "arch/x86_64/acpi.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "proc/process.h"
#include "sched/sched.h"
#include "drivers/pci.h"
#include <string.h>
#include <stdio.h>

extern void *_end;

static kernel_config_t _config;
static bool _kernel_running = false;
static volatile uint64_t _ticks = 0;

/* ── Built-in memory regions (from bootloader) ───────────────────── */

static memory_region_t early_regions[32];
static int early_region_count = 0;

static void build_early_regions(void) {
    uintptr_t kernel_end = (uintptr_t)&_end;
    early_regions[0].base = 0x100000;
    early_regions[0].length = 512ULL * 1024 * 1024;
    early_regions[0].type = PMM_REGION_USABLE;
    early_region_count = 1;
}

/* ── IRQ handlers ────────────────────────────────────────────────── */

static void pit_handler(uint8_t irq, interrupt_frame_t *frame) {
    (void)irq;
    (void)frame;
    _ticks++;
    sched_tick();
}

static void keyboard_handler(uint8_t irq, interrupt_frame_t *frame) {
    (void)irq;
    (void)frame;
    uint8_t scancode = inb(0x60);
    (void)scancode;
}

/* ── Init threads ────────────────────────────────────────────────── */

static void idle_thread(void *arg) {
    (void)arg;
    while (1) { __asm__ volatile("sti; hlt"); }
}

static void init_thread(void *arg) {
    (void)arg;
    printf("[INIT] OmniOS init process started\n");

    pci_init();

    printf("[INIT] All subsystems ready\n");
    printf("========================================\n");
    printf("  OmniOS v1.0 (x86_64 SMP) HAZIR\n");
    printf("  CPUs: %d  RAM: %llu MB\n",
        smp_online_cpus(),
        (unsigned long long)(pmm_get_total_memory() / 1024 / 1024));
    printf("========================================\n");

    while (1) { sched_sleep(1000); }
}

/* ── kmain (called from boot.S) ──────────────────────────────────── */

void kmain(uint32_t magic, uint32_t mbi) {
    (void)magic;
    (void)mbi;

    /* Early console */
    printf("\n=== OmniOS x86_64 ===\n");

    /* Architecture init */
    gdt_init();
    idt_init();
    paging_init();
    irq_init();

    /* Detect CPUs (BSP) */
    cpu_detect_all();

    /* ACPI */
    acpi_init();

    /* SMP boot APs */
    smp_init();

    /* APIC */
    apic_init();
    ioapic_init();

    /* Memory */
    build_early_regions();
    pmm_init(early_regions, early_region_count);
    vmm_init();

    /* Process + Scheduler */
    proc_init();
    sched_init();

    /* Timer (PIT → APIC timer) */
    irq_register(0, pit_handler);
    apic_timer_start(10000, LVT_TIMER, true);

    /* Input */
    irq_register(1, keyboard_handler);

    /* Create threads */
    sched_create_thread(0, idle_thread, NULL, 0);
    sched_create_thread(1, init_thread, NULL, 16);

    printf("[KERNEL] Starting scheduler...\n");
    irq_enable();

    /* Idle loop (scheduler runs via timer IRQ) */
    while (1) { __asm__ volatile("hlt"); }
}

/* ── Legacy kernel_init (kept for compatibility) ─────────────────── */

status_t kernel_init(const kernel_config_t *config) {
    memcpy(&_config, config, sizeof(kernel_config_t));
    _kernel_running = true;
    printf("[KERNEL] Legacy kernel_init called\n");
    return STATUS_SUCCESS;
}

/* ── Power management ────────────────────────────────────────────── */

void kernel_poweroff(void) {
    printf("[KERNEL] Power off\n");
    acpi_poweroff();
}

void kernel_reboot(void) {
    printf("[KERNEL] Reboot\n");
    acpi_reboot();
}

void kernel_panic(const char *message) {
    irq_disable();
    printf("\n=== KERNEL PANIC: %s ===\n", message ? message : "unknown");
    while (1) { __asm__ volatile("cli; hlt"); }
}

uint64_t kernel_get_ticks(void) { return _ticks; }

time_t kernel_get_uptime_ms(void) {
    return (time_t)(_ticks * 10);
}

