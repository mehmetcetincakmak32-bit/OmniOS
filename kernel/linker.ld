/* OmniOS — kernel/linker.ld */
/* x86_64 higher-half linker script — Multiboot2 + SMP */
/* SPDX-License-Identifier: MIT */

ENTRY(_start)

KERNEL_VMA = 0xFFFFFFFF80000000;
KERNEL_LMA = 0x00100000;

SECTIONS
{
    . = KERNEL_LMA;

    /* Multiboot header must be in first 8KB */
    .multiboot : {
        *(.multiboot)
    } :text

    /* Trampoline code at fixed low address for AP startup */
    . = ALIGN(4096);
    _trampoline_start = .;
    .trampoline : {
        *(.text.trampoline)
    }
    _trampoline_end = .;

    /* Main kernel at higher half */
    . += KERNEL_VMA - KERNEL_LMA;

    .text : AT(ADDR(.text) - KERNEL_VMA + KERNEL_LMA) {
        _code_start = .;
        *(.text._start)
        *(.text*)
        *(.text.*)
        _code_end = .;
    } :text

    . = ALIGN(4096);
    _rodata_start = .;
    .rodata : {
        *(.rodata*)
        *(.rodata.*)
    } :rodata
    _rodata_end = .;

    . = ALIGN(4096);
    _data_start = .;
    .data : {
        *(.data*)
        *(.data.*)
    } :data
    . = ALIGN(4096);
    .data.paging : {
        *(.data.paging)
    }
    _data_end = .;

    . = ALIGN(4096);
    _bss_start = .;
    .bss : {
        sbss = .;
        *(.bss*)
        *(COMMON)
        ebss = .;
    } :bss
    _bss_end = .;

    . = ALIGN(4096);
    _end = .;

    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
        *(.debug*)
    }
}

PHDRS
{
    text    PT_LOAD FLAGS(5);  /* R+X */
    rodata  PT_LOAD FLAGS(4);  /* R */
    data    PT_LOAD FLAGS(6);  /* R+W */
    bss     PT_LOAD FLAGS(6);  /* R+W */
}

