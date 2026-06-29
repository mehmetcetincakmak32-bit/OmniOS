/*
 * OmniOS Microkernel - Main Entry Point
 * 
 * This is the heart of OmniOS. It initializes all kernel subsystems
 * and starts the init process. Designed for ARM64 but architecture-
 * independent code is separated.
 */

#include "include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

/* Forward declarations of subsystem init functions */
static status_t _arch_init(void);
static status_t _console_init(void);
static void     _idle_thread(void* arg);
static void     _init_process(void* arg);
static void     _start_scheduler(void);

/* Kernel state */
static kernel_config_t _config;
static bool _kernel_running = false;
static time_t _boot_time = 0;
static char _panic_message[256];

/* Timer tick count */
static volatile uint64_t _ticks = 0;

/* ================================================================
 * Kernel Initialization
 * ================================================================ */

status_t kernel_init(const kernel_config_t* config) {
    if (!config) return STATUS_INVALID;

    /* Copy configuration */
    memcpy(&_config, config, sizeof(kernel_config_t));
    _boot_time = 0;
    _kernel_running = true;

    printf("[KERNEL] OmniOS v1.0 baslatiliyor...\n");
    printf("[KERNEL] Konfig: %dMB RAM, %d process, %d Hz tick\n",
           config->total_memory_mb, config->max_processes,
           config->ticks_per_second);

    /* Initialize subsystems in order */
    status_t status;

    status = _arch_init();
    if (status != STATUS_SUCCESS) {
        kernel_panic("Arch init failed");
        return status;
    }
    printf("[KERNEL] Arch katmani hazir\n");

    status = _console_init();
    if (status != STATUS_SUCCESS) {
        kernel_panic("Console init failed");
        return status;
    }
    printf("[KERNEL] Konsol hazir\n");

    status = mm_init(config->total_memory_mb);
    if (status != STATUS_SUCCESS) {
        kernel_panic("Memory manager init failed");
        return status;
    }
    printf("[KERNEL] Bellek yoneticisi hazir (%d MB)\n", config->total_memory_mb);

    status = sched_init();
    if (status != STATUS_SUCCESS) {
        kernel_panic("Scheduler init failed");
        return status;
    }
    printf("[KERNEL] Zamanlayici hazir (policy=%d, quantum=%dms)\n",
           config->sched_policy, config->quantum_ms);

    status = ipc_init();
    if (status != STATUS_SUCCESS) {
        kernel_panic("IPC init failed");
        return status;
    }
    printf("[KERNEL] IPC sistemi hazir\n");

    status = vfs_init();
    if (status != STATUS_SUCCESS) {
        kernel_panic("VFS init failed");
        return status;
    }
    printf("[KERNEL] VFS hazir\n");

    status = dev_init();
    if (status != STATUS_SUCCESS) {
        kernel_panic("Device manager init failed");
        return status;
    }
    printf("[KERNEL] Aygit surucusu modeli hazir\n");

    status = syscall_init();
    if (status != STATUS_SUCCESS) {
        kernel_panic("Syscall init failed");
        return status;
    }
    printf("[KERNEL] Sistem cagrilari hazir (%d adet)\n", SYS_MAX);

    if (config->enable_networking) {
        status = net_init();
        if (status != STATUS_SUCCESS) {
            printf("[KERNEL] UYARI: Ag baslatilamadi (%d)\n", status);
        } else {
            printf("[KERNEL] Ag yigini hazir\n");
        }
    }

    status = timer_init();
    if (status != STATUS_SUCCESS) {
        kernel_panic("Timer init failed");
        return status;
    }
    printf("[KERNEL] Zamanlayici servisi hazir\n");

    /* Create idle thread */
    tid_t idle_tid = sched_create_thread(0, _idle_thread, NULL, PRIORITY_MIN);
    if (idle_tid < 0) {
        kernel_panic("Idle thread creation failed");
        return STATUS_ERROR;
    }
    printf("[KERNEL] Idle thread olusturuldu (tid=%d)\n", idle_tid);

    /* Create init process */
    tid_t init_tid = sched_create_thread(1, _init_process, NULL, PRIORITY_DEFAULT);
    if (init_tid < 0) {
        kernel_panic("Init process creation failed");
        return STATUS_ERROR;
    }
    printf("[KERNEL] Init process olusturuldu (tid=%d)\n", init_tid);

    _boot_time = timer_get_uptime();

    printf("\n========================================\n");
    printf("  OmniOS v1.0 hazir\n");
    printf("  %d MB RAM | %d Hz | %d max proc\n",
           config->total_memory_mb, config->ticks_per_second,
           config->max_processes);
    printf("========================================\n\n");

    /* Start the scheduler - never returns */
    _start_scheduler();

    return STATUS_SUCCESS;
}

/* ================================================================
 * Kernel Lifecycle
 * ================================================================ */

void kernel_poweroff(void) {
    printf("[KERNEL] Sistem kapatiliyor...\n");
    _kernel_running = false;
    irq_disable();
    /* In real implementation: ACPI poweroff or PSCI call */
    while (1) { /* halt */ }
}

void kernel_reboot(void) {
    printf("[KERNEL] Sistem yeniden baslatiliyor...\n");
    _kernel_running = false;
    irq_disable();
    /* In real implementation: CPU reset */
    while (1) { /* halt */ }
}

void kernel_panic(const char* message) {
    irq_disable();
    if (message) {
        strncpy(_panic_message, message, sizeof(_panic_message) - 1);
    } else {
        strcpy(_panic_message, "unknown");
    }

    printf("\n========================================\n");
    printf("  KERNEL PANIC\n");
    printf("========================================\n");
    printf("  Message: %s\n", _panic_message);
    printf("  Ticks:   %llu\n", _ticks);
    printf("  PID:     %d\n", sched_get_current_pid());
    printf("  TID:     %d\n", sched_get_current_tid());
    printf("========================================\n");
    printf("  System halted.\n");

    while (1) { /* halt */ }
}

/* ================================================================
 * Architecture Layer
 * ================================================================ */

static status_t _arch_init(void) {
    /* In real implementation:
     *   - Set up page tables
     *   - Configure MMU
     *   - Set up interrupt vectors
     *   - Initialize GIC (ARM) or APIC (x86)
     *   - Set up system timer
     */
    return STATUS_SUCCESS;
}

static status_t _console_init(void) {
    /* In real implementation:
     *   - Initialize UART/serial console
     *   - Set up framebuffer
     */
    return STATUS_SUCCESS;
}

/* ================================================================
 * Idle Thread
 * ================================================================ */

static void _idle_thread(void* arg) {
    (void)arg;
    while (1) {
        /* Halt CPU until next interrupt */
        irq_enable();
        /* asm volatile("wfi"); // Wait For Interrupt */
        irq_disable();
    }
}

/* ================================================================
 * Init Process
 * ================================================================ */

static void _init_process(void* arg) {
    (void)arg;
    printf("[INIT] OmniOS init process basladi\n");

    /* Mount root filesystem */
    printf("[INIT] Kok dosya sistemi baglaniyor...\n");

    /* Start essential services */
    printf("[INIT] Servisler baslatiliyor...\n");
    printf("[INIT]  -> omniOS.ui\n");
    printf("[INIT]  -> omniOS.compatibility\n");
    printf("[INIT]  -> omniOS.gesture\n");

    /* Start system UI */
    printf("[INIT] Kullanici arayuzu baslatiliyor...\n");
    printf("[INIT] OmniOS Normal Mod hazir\n");

    /* In real implementation: load and execute /sbin/init or equivalent */
    while (1) {
        sched_sleep(1000);
    }
}

/* ================================================================
 * Scheduler Startup
 * ================================================================ */

static void _start_scheduler(void) {
    printf("[KERNEL] Zamanlayici baslatiliyor...\n");
    irq_enable();
    /* The scheduler takes over here */
    while (_kernel_running) {
        sched_yield();
    }
}

/* ================================================================
 * Timer Interrupt Handler
 * ================================================================ */

void timer_tick_handler(void) {
    _ticks++;
    sched_tick();
}

uint64_t kernel_get_ticks(void) {
    return _ticks;
}

time_t kernel_get_uptime_ms(void) {
    return (time_t)(_ticks * 1000 / _config.ticks_per_second);
}
