/* OmniOS — kernel/linker_arm64.ld */
/* ARM64 linker script — QEMU virt entry at 0x40080000 */
/* SPDX-License-Identifier: MIT */

ENTRY(_start)

KERNEL_LMA = 0x40080000;

SECTIONS
{
    . = KERNEL_LMA;

    .text : {
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

    /* initramfs embedded cpio archive */
    . = ALIGN(16);
    _initramfs_start = .;
    .initramfs : {
        *(.initramfs)
    }
    _initramfs_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.note.*)
        *(.eh_frame)
        *(.debug*)
    }
}

PHDRS
{
    text    PT_LOAD FLAGS(5);
    rodata  PT_LOAD FLAGS(4);
    data    PT_LOAD FLAGS(6);
    bss     PT_LOAD FLAGS(6);
}
