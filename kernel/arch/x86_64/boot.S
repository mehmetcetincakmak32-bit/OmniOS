/* OmniOS — kernel/arch/x86_64/boot.S */
/* x86_64 multi-core entry + SMP trampoline */
/* SPDX-License-Identifier: MIT */

#include <asm/multiboot.h>

/* Multiboot2 header — must be first in .text */
.section .multiboot, "a"
.align 8
.long MULTIBOOT2_HEADER_MAGIC
.long MULTIBOOT2_HEADER_FLAGS
.long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT2_HEADER_FLAGS)
/* Console flags tag */
.align 8
.word 4        /* type: console flags */
.word 1        /* flags */
.long 12       /* size */
.long 0        /* no EGA, text only */
.word 0        /* padding */
.word 0        /* padding */
/* End tag */
.align 8
.word 0        /* type: end */
.word 0        /* flags */
.long 8        /* size */

.section .text._start, "ax"
.code32
.global _start
_start:
    /* Save multiboot info */
    movl    %eax, %edi          /* magic in edi */
    movl    %ebx, %esi          /* mbi in esi */

    /* Quick check: long mode support via CPUID */
    movl    $0x80000000, %eax
    cpuid
    cmpl    $0x80000001, %eax
    jb      .Lhalt              /* no long mode -> halt */

    movl    $0x80000001, %eax
    cpuid
    testl   $(1 << 29), %edx    /* LM bit */
    jz      .Lhalt              /* no long mode -> halt */

    /* Enable PAE + PSE */
    movl    %cr4, %eax
    orl     $(1 << 5) | (1 << 4), %eax   /* PAE + PSE */
    movl    %eax, %cr4

    /* Load 4-level page table */
    leal    pml4 - KERNEL_VMA_BASE, %eax
    movl    %eax, %cr3

    /* Enable long mode via EFER MSR */
    movl    $0xC0000080, %ecx    /* EFER MSR */
    rdmsr
    orl     $(1 << 8), %eax      /* LME bit */
    wrmsr

    /* Enable paging (PE + PG) */
    movl    %cr0, %eax
    orl     $(1 << 31) | (1 << 0), %eax   /* PG | PE */
    movl    %eax, %cr0

    /* Load 64-bit GDT */
    lgdt    gdt64_ptr - KERNEL_VMA_BASE

    /* Far jump to 64-bit mode */
    ljmp    $0x08, $.Lentry64 - KERNEL_VMA_BASE

.code64
.Lentry64:
    /* Set up seg registers */
    movw    $0x10, %ax          /* data segment */
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %fs
    movw    %ax, %gs
    movw    %ax, %ss

    /* Clear BSS */
    movq    $sbss, %rax
    movq    $ebss, %rdi
    subq    %rax, %rdi
    xorl    %eax, %eax
    shrq    $3, %rcx
    rep stosq

    /* Set up kernel stack (per-CPU) */
    leaq    _cpu0_stack_top, %rsp

    /* Push multiboot info for kmain */
    movl    %edi, %edi          /* zero-extend magic */
    movl    %esi, %esi          /* zero-extend mbi */

    /* Jump to kmain */
    call    kmain

    /* Should never return */
.Lhalt:
    cli
    hlt
    jmp     .Lhalt

/* ── SMP AP trampoline (runs on each AP core) ────────────────────── */

.section .text.trampoline, "ax"
.code16
.global ap_trampoline_start
.global ap_trampoline_end
ap_trampoline_start:
    /* Real-mode entry for SIPI */
    cli
    cld
    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

    /* Set up GDT for protected mode */
    lgdt    ap_gdt_ptr - ap_trampoline_start

    /* Enable protected mode */
    movl    %cr0, %eax
    orl     $1, %eax
    movl    %eax, %cr0

    /* Far jump to 32-bit */
    ljmp    $0x08, $ap_entry32 - ap_trampoline_start

.code32
ap_entry32:
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

    /* Enable PAE */
    movl    %cr4, %eax
    orl     $(1 << 5), %eax
    movl    %eax, %cr4

    /* Load shared page tables */
    movl    (pml4 - KERNEL_VMA_BASE), %eax
    movl    %eax, %cr3

    /* Enable long mode */
    movl    $0xC0000080, %ecx
    rdmsr
    orl     $(1 << 8), %eax
    wrmsr

    /* Enable paging */
    movl    %cr0, %eax
    orl     $(1 << 31) | $(1 << 0), %eax
    movl    %eax, %cr0

    /* 64-bit! */
    lgdt    gdt64_ptr - KERNEL_VMA_BASE
    ljmp    $0x08, $ap_entry64 - KERNEL_VMA_BASE

.code64
ap_entry64:
    movw    $0x10, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

    /* Get APIC ID to index per-CPU stack */
    movl    $1, %eax
    cpuid
    shrl    $24, %ebx           /* APIC ID in EBX[31:24] */
    movl    %ebx, %edi

    /* Look up per-CPU stack */
    leaq    _cpu_stacks, %rsi
    movq    (%rsi, %rdi, 8), %rsp

    /* Call AP entry */
    call    ap_entry
.Lap_halt:
    cli
    hlt
    jmp     .Lap_halt

.align 16
ap_gdt_ptr:
    .word   gdt64_end - gdt64 - 1
    .long   gdt64 - ap_trampoline_start

ap_trampoline_end:
    nop

/* ── GDT (64-bit) ────────────────────────────────────────────────── */

.section .rodata
.align 16
gdt64:
    .quad 0x0000000000000000   /* NULL descriptor */
    .quad 0x00209A0000000000   /* 64-bit code segment (ring 0) */
    .quad 0x0000920000000000   /* 64-bit data segment (ring 0) */
    .quad 0x0020FA0000000000   /* 64-bit code segment (ring 3) */
    .quad 0x0000F20000000000   /* 64-bit data segment (ring 3) */
    .quad 0x0000000000000000   /* TSS low */
    .quad 0x0000000000000000   /* TSS high */
gdt64_end:

gdt64_ptr:
    .word gdt64_end - gdt64 - 1
    .quad gdt64

/* ── Page tables (identity map first 4GB + higher half) ──────────── */

.section .data.paging, "aw", @nobits
.align 4096
pml4:
    .skip 4096
pdp:
    .skip 4096
pd:
    .skip 4096
pt:
    .skip 4096

/* ── BSS ─────────────────────────────────────────────────────────── */

.section .bss
.align 16
sbss:
_cpu0_stack:
    .skip 16384
_cpu0_stack_top:

/* Per-CPU stacks (up to 256 CPUs) */
.align 16
_cpu_stacks:
    .skip 8 * 256

ebss:
