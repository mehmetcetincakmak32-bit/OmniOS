/* OmniOS — kernel/arch/x86_64/acpi.h */
/* ACPI table parser — MADT, FADT, DSDT, shutdown */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_ACPI_H
#define OMNIOS_ACPI_H

#include <stdint.h>
#include <stdbool.h>

/* ── RSDP / RSDT / XSDT ──────────────────────────────────────────── */

typedef struct {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_addr;
} __attribute__((packed)) rsdp_t;

typedef struct {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
    acpi_sdt_header_t header;
    uint32_t entry[];
} __attribute__((packed)) rsdt_t;

/* ── MADT (Multiple APIC Description Table) ──────────────────────── */

typedef struct {
    acpi_sdt_header_t header;
    uint32_t          lapic_addr;
    uint32_t          flags;
} __attribute__((packed)) madt_t;

typedef struct {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) madt_entry_header_t;

#define MADT_LAPIC      0
#define MADT_IOAPIC     1
#define MADT_ISO        2
#define MADT_NMI        4
#define MADT_LAPIC_O   5

/* ── FADT (Fixed ACPI Description Table) ─────────────────────────── */

typedef struct {
    acpi_sdt_header_t header;
    uint32_t          facs_addr;
    uint32_t          dsdt_addr;
    uint8_t           model;
    uint8_t           reserved1;
    uint16_t          sci_int;
    uint32_t          smi_cmd;
    uint8_t           acpi_enable;
    uint8_t           acpi_disable;
    uint8_t           s4bios_req;
    uint8_t           pstate_cnt;
    uint32_t          pm1a_evt_blk;
    uint32_t          pm1b_evt_blk;
    uint32_t          pm1a_cnt_blk;
    uint32_t          pm1b_cnt_blk;
    uint32_t          pm2_cnt_blk;
    uint32_t          pm_tmr_blk;
    uint32_t          gpe0_blk;
    uint32_t          gpe1_blk;
    uint8_t           pm1_evt_len;
    uint8_t           pm1_cnt_len;
    uint8_t           pm2_cnt_len;
    uint8_t           pm_tmr_len;
    uint8_t           gpe0_len;
    uint8_t           gpe1_len;
    uint8_t           gpe1_base;
    uint8_t           reserved2;
    uint16_t          p_lvl2_lat;
    uint16_t          p_lvl3_lat;
    uint16_t          flush_size;
    uint16_t          flush_stride;
    uint8_t           duty_offset;
    uint8_t           duty_width;
    uint8_t           day_alrm;
    uint8_t           mon_alrm;
    uint8_t           century;
    uint16_t          iapc_boot_arch;
    uint8_t           reserved3;
    uint32_t          flags;
} __attribute__((packed)) fadt_t;

/* ── Public API ──────────────────────────────────────────────────── */

int  acpi_init(void);
void acpi_poweroff(void);
void acpi_reboot(void);
bool acpi_get_madt(uintptr_t *addr, size_t *size);
uint32_t acpi_get_lapic_count(void);
uint32_t acpi_get_ioapic_count(void);

#endif /* OMNIOS_ACPI_H */
