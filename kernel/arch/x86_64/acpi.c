/* OmniOS — kernel/arch/x86_64/acpi.c */
/* ACPI table parser */
/* SPDX-License-Identifier: MIT */

#include "acpi.h"
#include "io.h"
#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

static rsdp_t *rsdp = NULL;
static fadt_t *fadt = NULL;

/* EBDA search + BIOS area */
static uintptr_t find_rsdp(void) {
    /* Search EBDA (0x40E) */
    uint16_t ebda_seg;
    memcpy(&ebda_seg, (void *)0x40E, 2);
    uintptr_t ebda = (uintptr_t)ebda_seg << 4;

    for (uintptr_t addr = ebda; addr < ebda + 0x400; addr += 16) {
        if (memcmp((void *)addr, "RSD PTR ", 8) == 0) {
            return addr;
        }
    }
    /* Search BIOS area */
    for (uintptr_t addr = 0xE0000; addr < 0x100000; addr += 16) {
        if (memcmp((void *)addr, "RSD PTR ", 8) == 0) {
            return addr;
        }
    }
    return 0;
}

static bool acpi_checksum(void *table, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) sum += ((uint8_t *)table)[i];
    return sum == 0;
}

static void *find_table(const char *sig) {
    if (!rsdp) return NULL;
    rsdt_t *rsdt = (rsdt_t *)(uintptr_t)rsdp->rsdt_addr;
    if (!acpi_checksum(rsdt, rsdt->header.length)) return NULL;

    int count = (rsdt->header.length - sizeof(acpi_sdt_header_t)) / 4;
    for (int i = 0; i < count; i++) {
        acpi_sdt_header_t *hdr = (acpi_sdt_header_t *)(uintptr_t)rsdt->entry[i];
        if (memcmp(hdr->signature, sig, 4) == 0) {
            if (acpi_checksum(hdr, hdr->length)) return hdr;
        }
    }
    return NULL;
}

int acpi_init(void) {
    uintptr_t rsdp_addr = find_rsdp();
    if (!rsdp_addr) {
        printf("[ACPI] RSDP not found\n");
        return -1;
    }
    rsdp = (rsdp_t *)rsdp_addr;
    printf("[ACPI] RSDP @ 0x%llx, revision %d\n",
        (unsigned long long)rsdp_addr, rsdp->revision);

    fadt = (fadt_t *)find_table("FACP");
    if (!fadt) {
        printf("[ACPI] FADT not found\n");
        return -1;
    }
    printf("[ACPI] FADT @ %p, DSDT @ 0x%x\n", (void *)fadt, fadt->dsdt_addr);

    /* Parse MADT for APIC info */
    madt_t *madt = (madt_t *)find_table("APIC");
    if (madt) {
        printf("[ACPI] MADT: lapic_addr=0x%x flags=0x%x\n",
            madt->lapic_addr, madt->flags);
        uintptr_t entry_start = (uintptr_t)madt + sizeof(madt_t);
        uintptr_t entry_end = (uintptr_t)madt + madt->header.length;
        uint32_t lapic_count = 0, ioapic_count = 0;

        for (uintptr_t p = entry_start; p < entry_end; ) {
            madt_entry_header_t *h = (madt_entry_header_t *)p;
            if (h->type == MADT_LAPIC) lapic_count++;
            else if (h->type == MADT_IOAPIC) ioapic_count++;
            p += h->length;
        }
        printf("[ACPI] MADT: %u LAPICs, %u I/O APICs\n", lapic_count, ioapic_count);
    }
    return 0;
}

void acpi_poweroff(void) {
    if (!fadt) return;
    uint16_t pm1a_cnt = fadt->pm1a_cnt_blk;
    uint16_t slp_typa = 0;
    uint16_t slp_typb = 0;

    /* PM1a_CNT: SLP_TYP = 5 (S5), SLP_EN = 1 */
    outw(pm1a_cnt, slp_typa | (5 << 10) | (1 << 13));

    /* Fallback: triple fault */
    __asm__ volatile("cli; hlt");
}

void acpi_reboot(void) {
    (void)fadt; /* reset_reg in FADT v2+; use KBC fallback */
    outb(0x64, 0xFE);
    __asm__ volatile("cli; hlt");
}

bool acpi_get_madt(uintptr_t *addr, size_t *size) {
    madt_t *m = (madt_t *)find_table("APIC");
    if (!m) return false;
    *addr = (uintptr_t)m;
    *size = m->header.length;
    return true;
}

uint32_t acpi_get_lapic_count(void) {
    uintptr_t addr;
    size_t size;
    if (!acpi_get_madt(&addr, &size)) return 0;
    uint32_t count = 0;
    uintptr_t end = addr + size;
    addr += sizeof(madt_t);
    while (addr < end) {
        madt_entry_header_t *h = (madt_entry_header_t *)addr;
        if (h->type == MADT_LAPIC) count++;
        addr += h->length;
    }
    return count;
}

uint32_t acpi_get_ioapic_count(void) {
    uintptr_t addr;
    size_t size;
    if (!acpi_get_madt(&addr, &size)) return 0;
    uint32_t count = 0;
    uintptr_t end = addr + size;
    addr += sizeof(madt_t);
    while (addr < end) {
        madt_entry_header_t *h = (madt_entry_header_t *)addr;
        if (h->type == MADT_IOAPIC) count++;
        addr += h->length;
    }
    return count;
}
