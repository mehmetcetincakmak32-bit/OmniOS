/*
 * OmniOS Kernel — Main Entry Point
 * x86_64 SMP / ARM64 Mobile initialization
 */

#include "include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

#if defined(__x86_64__) || defined(CONFIG_X86_64)
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
#define ARCH_X86_64
#elif defined(__aarch64__) || defined(CONFIG_ARM64)
#include "arch/arm64/arm64.h"
#include "arch/arm64/exceptions.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "proc/process.h"
#include "sched/sched.h"
#include "klog.h"
#include "syscall.h"
#include "fs/devfs.h"
#include "fs/procfs.h"
#include "fs/elf.h"
#include "fs/initramfs.h"
#include "drivers/tty.h"
#include "bsp/sm8250/sm8250.h"
#include "bsp/sm8250/drivers/modem_ril.h"
#include "bsp/sm8250/drivers/sensors.h"
#include "bsp/sm8250/drivers/power_mgmt.h"
#include "bsp/sm8250/drivers/usb_otg.h"
#include "bsp/sm8250/drivers/wifi_bt.h"
#include "bsp/sm8250/drivers/init.h"
#include "bsp/sm8250/drivers/display_fb.h"
#define ARCH_ARM64
#endif

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
#ifdef ARCH_ARM64
    while (1) { __asm__ volatile("wfi"); }
#else
    while (1) { __asm__ volatile("sti; hlt"); }
#endif
}

static void init_thread(void *arg) {
    (void)arg;
    printf("[INIT] OmniOS init process started\n");

#ifdef ARCH_X86_64
    pci_init();
    printf("[INIT] All subsystems ready\n");
    printf("========================================\n");
    printf("  OmniOS v1.0 (x86_64 SMP) HAZIR\n");
    printf("  CPUs: %d  RAM: %llu MB\n",
        smp_online_cpus(),
        (unsigned long long)(pmm_get_total_memory() / 1024 / 1024));
    printf("========================================\n");
#elif defined(ARCH_ARM64)
    mobile_boot_animation();
    mobile_boot_init();

    /* Init kernel subsystems */
    devfs_init();
    procfs_init();
    tty_init();

    /* Try to load and execute init from initramfs */
    extern uint8_t _initramfs_start[];
    extern uint8_t _initramfs_end[];
    size_t initramfs_size = _initramfs_end - _initramfs_start;

    if (initramfs_size > 0) {
        printf("[INIT] initramfs: %zu bytes\n", initramfs_size);
        syscall_set_initramfs(_initramfs_start, initramfs_size);

        /* execve init */
        long ret = syscall_dispatch(9, (long)"init", 0, 0, 0, 0, 0);
        printf("[INIT] execve returned: %ld\n", ret);
    } else {
        printf("[INIT] No initramfs embedded\n");
    }
#endif

    while (1) { sched_sleep(1000); }
}

/* ── kmain (called from boot.S) ──────────────────────────────────── */

void kmain(uint32_t magic, uint32_t mbi) {
    (void)magic;
    (void)mbi;

#ifdef ARCH_ARM64
    printf("\n=== OmniOS ARM64 Mobile ===\n");

    klog_init();
    syscall_init();
    arm64_paging_init();
    arm64_exception_init();

    sm8250_init();

    build_early_regions();
    pmm_init(early_regions, early_region_count);
    vmm_init();

    proc_init();
    sched_init();

    arm64_timer_init(arm64_read_cntfrq());

    sched_create_thread(0, idle_thread, NULL, 0);
    sched_create_thread(1, init_thread, NULL, 16);

    printf("[KERNEL] Starting scheduler...\n");
    irq_enable();

    while (1) { __asm__ volatile("wfi"); }
#else
    printf("\n=== OmniOS x86_64 ===\n");

    gdt_init();
    idt_init();
    paging_init();
    irq_init();

    cpu_detect_all();
    acpi_init();
    smp_init();
    apic_init();
    ioapic_init();

    build_early_regions();
    pmm_init(early_regions, early_region_count);
    vmm_init();

    proc_init();
    sched_init();

    irq_register(0, pit_handler);
    apic_timer_start(10000, LVT_TIMER, true);
    irq_register(1, keyboard_handler);

    sched_create_thread(0, idle_thread, NULL, 0);
    sched_create_thread(1, init_thread, NULL, 16);

    printf("[KERNEL] Starting scheduler...\n");
    irq_enable();

    while (1) { __asm__ volatile("hlt"); }
#endif
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
#ifdef ARCH_ARM64
    sm8250_poweroff();
#else
    acpi_poweroff();
#endif
}

void kernel_reboot(void) {
    printf("[KERNEL] Reboot\n");
#ifdef ARCH_ARM64
    sm8250_reboot();
#else
    acpi_reboot();
#endif
}

void kernel_panic(const char *message) {
    irq_disable();
    printf("\n=== KERNEL PANIC: %s ===\n", message ? message : "unknown");
#ifdef ARCH_ARM64
    while (1) { __asm__ volatile("wfi"); }
#else
    while (1) { __asm__ volatile("cli; hlt"); }
#endif
}

uint64_t kernel_get_ticks(void) { return _ticks; }

time_t kernel_get_uptime_ms(void) {
    return (time_t)(_ticks * 10);
}

