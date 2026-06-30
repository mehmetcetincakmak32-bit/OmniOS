/* OmniOS — kernel/arch/arm64/boot.S */
/* ARM64 EFI/DeviceTree entry + SMP spin-table */
/* SPDX-License-Identifier: MIT */

.section .text._start, "ax"
.global _start
_start:
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #2
    b.ne    1f
    mov     x0, #(1 << 31)
    msr     hcr_el2, x0
    mov     x0, #0x3C5
    msr     spsr_el2, x0
    adr     x0, 1f
    msr     elr_el2, x0
    eret
1:
    ldr     x0, =_cpu0_stack_top
    mov     sp, x0
    ldr     x0, =sbss
    ldr     x1, =ebss
2:  cmp     x0, x1
    b.ge    3f
    str     xzr, [x0], #8
    b       2b
3:
    mov     x19, x0
    mrs     x0, cpacr_el1
    orr     x0, x0, #(3 << 20)
    msr     cpacr_el1, x0
    bl      arm64_paging_init
    mov     x0, x19
    bl      kmain
.Lhalt:
    wfi
    b       .Lhalt

.section .text.cpu_entry, "ax"
.global _secondary_entry
_secondary_entry:
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    ldr     x1, =_cpu_stacks
    ldr     x2, [x1, x0, lsl #3]
    mov     sp, x2
    bl      arm64_cpu_init
    msr     daifclr, #0xF
    b       .Lhalt

.section .data.paging, "aw", @progbits
.align 12
.global arm64_pgtables
arm64_pgtables:
    .skip 0x4000

.section .bss
.align 4
sbss:
_cpu0_stack:
    .skip 32768
_cpu0_stack_top:
_cpu_stacks:
    .skip 8 * 256
ebss:
