/* OmniOS — kernel/drivers/pci.c */
/* PCI bus enumeration — config space access */
/* SPDX-License-Identifier: MIT */

#include "pci.h"
#include "../arch/x86_64/io.h"
#include <stdio.h>
#include <string.h>

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static pci_device_t devices[PCI_MAX_DEVICES];
static int device_count = 0;

static uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func,
        uint8_t offset) {
    uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) |
        (offset & 0xFC);
    outl(PCI_CONFIG_ADDR, addr);
    return inl(PCI_CONFIG_DATA);
}

static void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func,
        uint8_t offset, uint32_t value) {
    uint32_t addr = (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) |
        (offset & 0xFC);
    outl(PCI_CONFIG_ADDR, addr);
    outl(PCI_CONFIG_DATA, value);
}

static void pci_enable_bus_master(pci_device_t *dev) {
    uint32_t cmd = pci_read_config(dev->bus, dev->slot, dev->func, PCI_COMMAND);
    cmd |= PCI_COMMAND_IO | PCI_COMMAND_MEM | PCI_COMMAND_MASTER;
    pci_write_config(dev->bus, dev->slot, dev->func, PCI_COMMAND, cmd);
    dev->command = cmd;
}

static uint64_t pci_read_bar(pci_device_t *dev, int bar) {
    uint8_t offset = PCI_BAR0 + bar * 4;
    uint32_t low = pci_read_config(dev->bus, dev->slot, dev->func, offset);

    if (low & 1) {
        /* I/O bar */
        return low & ~3;
    }
    if ((low & 6) == 4) {
        /* 64-bit MMIO bar */
        uint32_t high = pci_read_config(dev->bus, dev->slot, dev->func, offset + 4);
        return ((uint64_t)high << 32) | (low & ~0xF);
    }
    return low & ~0xF;
}

static void pci_scan_bus(uint8_t bus) {
    for (int slot = 0; slot < 32; slot++) {
        uint32_t id = pci_read_config(bus, slot, 0, 0);
        if (id == 0xFFFFFFFF) continue;

        uint8_t hdr_type = (pci_read_config(bus, slot, 0, PCI_HEADER_TYPE) >> 16) & 0xFF;

        for (int func = 0; func < (hdr_type & 0x80 ? 8 : 1); func++) {
            uint32_t id2 = pci_read_config(bus, slot, func, 0);
            if (id2 == 0xFFFFFFFF) continue;

            if (device_count >= PCI_MAX_DEVICES) return;
            pci_device_t *dev = &devices[device_count++];
            memset(dev, 0, sizeof(*dev));

            dev->vendor_id = id2 & 0xFFFF;
            dev->device_id = id2 >> 16;
            dev->bus = bus;
            dev->slot = slot;
            dev->func = func;
            dev->valid = true;

            uint32_t class_rev = pci_read_config(bus, slot, func, PCI_CLASS);
            dev->revision = class_rev & 0xFF;
            dev->class = (class_rev >> 24) & 0xFF;
            dev->subclass = (class_rev >> 16) & 0xFF;
            dev->prog_if = (class_rev >> 8) & 0xFF;

            uint32_t cmd = pci_read_config(bus, slot, func, PCI_COMMAND);
            dev->command = cmd & 0xFFFF;

            for (int b = 0; b < 6; b++) dev->bars[b] = pci_read_bar(dev, b);

            uint32_t irq_line = pci_read_config(bus, slot, func, PCI_INTERRUPT);
            dev->irq = irq_line & 0xFF;

            pci_enable_bus_master(dev);

            printf("  PCI %02x:%02x.%x %04x:%04x class=%02x subclass=%02x irq=%d\n",
                bus, slot, func, dev->vendor_id, dev->device_id,
                dev->class, dev->subclass, dev->irq);
        }
    }
}

void pci_init(void) {
    device_count = 0;
    printf("[PCI] Scanning PCI bus...\n");
    pci_scan_bus(0);

    /* Check for PCIe (bus 0, device 0, func 0, header type) */
    uint32_t hdr = pci_read_config(0, 0, 0, PCI_HEADER_TYPE);
    if ((hdr >> 16) & 0x80) {
        /* Multi-function at dev 0, check for PCIe host bridge */
        uint32_t vid = pci_read_config(0, 0, 0, 0);
        if (vid != 0xFFFFFFFF) {
            pci_scan_bus(0);
        }
    }
    printf("[PCI] %d devices found\n", device_count);
}

int pci_scan(void) { return device_count; }

pci_device_t *pci_find(uint16_t vendor, uint16_t device) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].vendor_id == vendor && devices[i].device_id == device)
            return &devices[i];
    }
    return NULL;
}

pci_device_t *pci_find_class(uint8_t cls, uint8_t subclass) {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].class == cls && devices[i].subclass == subclass)
            return &devices[i];
    }
    return NULL;
}

int pci_device_count(void) { return device_count; }

pci_device_t *pci_get_device(int index) {
    if (index < 0 || index >= device_count) return NULL;
    return &devices[index];
}
