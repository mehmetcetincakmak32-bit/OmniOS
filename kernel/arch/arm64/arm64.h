/* OmniOS — kernel/arch/arm64/arm64.h */
/* ARM64 arch definitions */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_ARM64_H
#define OMNIOS_ARM64_H

#include <stdint.h>
#include <stddef.h>

/* Generic Timer */
#define CNTFRQ_EL0      0xDFE0
#define CNTPCT_EL0      0xDFE1
#define CNTP_CVAL_EL0   0xDFE2
#define CNTP_TVAL_EL0   0xDFE4
#define CNTP_CTL_EL0    0xDFE5

/* GICv3 base addresses (QEMU virt) */
#define GICD_BASE       0x08000000
#define GICR_BASE       0x080A0000
#define GICC_BASE       0x80100000

#define GICD_CTLR       0x0000
#define GICD_TYPER      0x0004
#define GICC_IAR        0x000C
#define GICC_EOIR       0x0010
#define GICC_PMR        0x0004

/* System register access */
static inline uint64_t arm64_read_cntfrq(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(val));
    return val;
}

static inline uint64_t arm64_read_cntpct(void) {
    uint64_t val;
    __asm__ volatile("mrs %0, cntpct_el0" : "=r"(val));
    return val;
}

static inline void arm64_write_cntp_tval(uint32_t val) {
    __asm__ volatile("msr cntp_tval_el0, %0" : : "r"(val));
}

static inline void arm64_write_cntp_ctl(uint32_t val) {
    __asm__ volatile("msr cntp_ctl_el0, %0" : : "r"(val));
}

/* MMIO accessors */
static inline uint32_t gic_read32(uintptr_t base, uint32_t reg) {
    return *(volatile uint32_t *)(base + reg);
}
static inline void gic_write32(uintptr_t base, uint32_t reg, uint32_t val) {
    *(volatile uint32_t *)(base + reg) = val;
}

/* Page size constant */
#define PAGE_SIZE  4096

/* API */
void arm64_gic_init(void);
void arm64_gic_eoi(uint32_t irq);
void arm64_timer_init(uint32_t freq_hz);
void arm64_paging_init(void);
void arm64_cpu_init(void);

/* User page table management */
uintptr_t arm64_create_user_pgd(void);
void      arm64_map_user_page(uintptr_t pgd, uintptr_t virt, uintptr_t phys, int writable, int executable);
void      arm64_map_user_pages(uintptr_t pgd, uintptr_t virt, uintptr_t phys, size_t pages, int writable, int executable);
void      arm64_set_ttbr0(uintptr_t pgd);

#endif /* OMNIOS_ARM64_H */
