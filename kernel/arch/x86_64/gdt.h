/* OmniOS — kernel/arch/x86_64/gdt.h */
/* x86_64 GDT with per-CPU TSS for IST */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_GDT_H
#define OMNIOS_GDT_H

#include <stdint.h>

#define GDT_ENTRIES 64

/* Segment selectors */
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   0x1B  /* 0x18 | 3 (RPL) */
#define GDT_USER_DATA   0x23  /* 0x20 | 3 */
#define GDT_TSS_BASE    0x28  /* First TSS entry (64-bit TSS descriptor takes 2 slots) */

void gdt_init(void);
void gdt_load_tss(uint32_t cpu_index, uintptr_t tss_addr, size_t tss_size);

#endif /* OMNIOS_GDT_H */
