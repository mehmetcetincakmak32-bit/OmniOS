/* OmniOS — kernel/arch/x86_64/idt.h */
/* x86_64 IDT + exception vectors */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_IDT_H
#define OMNIOS_IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

/* IST indices (1-based) */
#define IST_DOUBLE_FAULT 1
#define IST_NMI          2
#define IST_MCE          3

/* Exception vectors */
#define INT_DIVIDE_ERROR    0
#define INT_DEBUG           1
#define INT_NMI             2
#define INT_BREAKPOINT      3
#define INT_OVERFLOW        4
#define INT_BOUND_RANGE     5
#define INT_INVALID_OPCODE  6
#define INT_DEVICE_NA       7
#define INT_DOUBLE_FAULT    8
#define INT_COPROCESSOR     9  /* x87 FPU (legacy) */
#define INT_INVALID_TSS    10
#define INT_SEGMENT_NP     11
#define INT_STACK_FAULT    12
#define INT_GP_FAULT       13
#define INT_PAGE_FAULT     14
#define INT_RESERVED       15
#define INT_FPU_ERROR      16
#define INT_ALIGNMENT      17
#define INT_MCE            18
#define INT_SIMD_FP        19
#define INT_VIRT_ERROR     20
#define INT_CONTROL_PROT   21  /* Control-flow Protection */
#define INT_CPL_HV         22
#define INT_CPL_VMM        23
#define INT_CPL_VMEXIT     24

/* Hardware IRQ offsets */
#define IRQ_BASE            0x20
#define IRQ_PIT             0x20  /* IRQ 0 */
#define IRQ_KEYBOARD        0x21  /* IRQ 1 */
#define IRQ_CASCADE         0x22  /* IRQ 2 (slave PIC) */
#define IRQ_COM2            0x23  /* IRQ 3 */
#define IRQ_COM1            0x24  /* IRQ 4 */
#define IRQ_LPT2            0x25  /* IRQ 5 */
#define IRQ_FLOPPY          0x26  /* IRQ 6 */
#define IRQ_LPT1            0x27  /* IRQ 7 */
#define IRQ_CMOS_RTC        0x28  /* IRQ 8 */
#define IRQ_ACPI            0x29  /* IRQ 9 */
#define IRQ_SCI             IRQ_ACPI
#define IRQ_AVAILABLE       0x2A  /* IRQ 10 */
#define IRQ_NIC             0x2B  /* IRQ 11 */
#define IRQ_PS2_MOUSE       0x2C  /* IRQ 12 */
#define IRQ_FPU             0x2D  /* IRQ 13 */
#define IRQ_PRIMARY_ATA     0x2E  /* IRQ 14 */
#define IRQ_SECONDARY_ATA   0x2F  /* IRQ 15 */

/* Local APIC LVT interrupts (x2APIC) */
#define LVT_TIMER           0x32
#define LVT_THERMAL         0x33
#define LVT_PERFMON         0x34
#define LVT_LINT0           0x35
#define LVT_LINT1           0x36
#define LVT_ERROR           0x37
#define IPI_VECTOR          0x3F

typedef struct {
    uint16_t    offset_low;
    uint16_t    selector;
    uint8_t     ist;
    uint8_t     type_attr;
    uint16_t    offset_mid;
    uint32_t    offset_high;
    uint32_t    reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr_t;

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t vector, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) interrupt_frame_t;

/* Interrupt handler type */
typedef void (*interrupt_handler_t)(interrupt_frame_t *frame);

void idt_init(void);
void idt_set_handler(uint8_t vec, interrupt_handler_t handler,
        uint8_t ist, bool user);
void idt_install(void);

/* Exception handler names for debugging */
extern const char *exception_names[];

#endif /* OMNIOS_IDT_H */
