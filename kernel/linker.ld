/*
 * OmniOS Kernel Linker Script
 */

ENTRY(_start)

SECTIONS
{
    . = 0x80000;  /* Kernel load address */

    .text : {
        *(.text._start)
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }

    . = ALIGN(4096);
    _end = .;
}
