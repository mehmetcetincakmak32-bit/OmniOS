/* OmniOS — kernel/arch/x86_64/io.h */
/* x86_64 port I/O + MMIO helpers */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_IO_H
#define OMNIOS_IO_H

#include <stdint.h>

/* ── Port I/O ────────────────────────────────────────────────────── */

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}
static inline uint16_t inw(uint16_t port) {
    uint16_t val;
    __asm__ volatile("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}
static inline uint32_t inl(uint16_t port) {
    uint32_t val;
    __asm__ volatile("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

/* ── IO delay (for legacy device timing) ─────────────────────────── */

static inline void io_wait(void) {
    outb(0x80, 0);
}

/* ── MMIO read/write with volatile + memory barrier ──────────────── */

static inline uint8_t  mmio_read8(volatile void *addr) {
    __sync_synchronize();
    return *(volatile uint8_t *)addr;
}
static inline uint16_t mmio_read16(volatile void *addr) {
    __sync_synchronize();
    return *(volatile uint16_t *)addr;
}
static inline uint32_t mmio_read32(volatile void *addr) {
    __sync_synchronize();
    return *(volatile uint32_t *)addr;
}
static inline uint64_t mmio_read64(volatile void *addr) {
    __sync_synchronize();
    return *(volatile uint64_t *)addr;
}
static inline void mmio_write8(volatile void *addr, uint8_t val) {
    *(volatile uint8_t *)addr = val;
    __sync_synchronize();
}
static inline void mmio_write16(volatile void *addr, uint16_t val) {
    *(volatile uint16_t *)addr = val;
    __sync_synchronize();
}
static inline void mmio_write32(volatile void *addr, uint32_t val) {
    *(volatile uint32_t *)addr = val;
    __sync_synchronize();
}
static inline void mmio_write64(volatile void *addr, uint64_t val) {
    *(volatile uint64_t *)addr = val;
    __sync_synchronize();
}

#endif /* OMNIOS_IO_H */
