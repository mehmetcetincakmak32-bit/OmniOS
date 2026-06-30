/* OmniOS — kernel/drivers/pci.h */
/* PCI/PCIe bus enumeration */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_PCI_H
#define OMNIOS_PCI_H

#include <stdint.h>
#include <stdbool.h>

#define PCI_MAX_DEVICES 256
#define PCI_VENDOR_ID   0x00
#define PCI_DEVICE_ID   0x02
#define PCI_COMMAND     0x04
#define PCI_STATUS      0x06
#define PCI_REVISION    0x08
#define PCI_CLASS       0x0A
#define PCI_HEADER_TYPE 0x0E
#define PCI_BAR0        0x10
#define PCI_BAR1        0x14
#define PCI_BAR2        0x18
#define PCI_BAR3        0x1C
#define PCI_BAR4        0x20
#define PCI_BAR5        0x24
#define PCI_CAP_PTR     0x34
#define PCI_INTERRUPT   0x3C

#define PCI_COMMAND_IO        (1 << 0)
#define PCI_COMMAND_MEM       (1 << 1)
#define PCI_COMMAND_MASTER    (1 << 2)
#define PCI_COMMAND_INTX_DIS  (1 << 10)

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t  revision;
    uint8_t  class;
    uint8_t  subclass;
    uint8_t  prog_if;
    uint8_t  bus;
    uint8_t  slot;
    uint8_t  func;
    uint64_t bars[6];
    uint8_t  irq;
    bool     valid;
} pci_device_t;

void pci_init(void);
int pci_scan(void);
pci_device_t *pci_find(uint16_t vendor, uint16_t device);
pci_device_t *pci_find_class(uint8_t class, uint8_t subclass);
int pci_device_count(void);
pci_device_t *pci_get_device(int index);

#endif /* OMNIOS_PCI_H */
