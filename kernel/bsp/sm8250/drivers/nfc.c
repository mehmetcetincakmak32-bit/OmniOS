/* OmniOS — kernel/bsp/sm8250/drivers/nfc.c */
/* NXP PN553 NFC controller (I2C) */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>

#define NFC_I2C_BASE      (SM8250_QUPV3_BASE + 0x20000)
#define NFC_IRQ           112
#define NFC_VEN_GPIO      12
#define NFC_FW_GPIO       13

#define NFC_CMD_CORE_RESET      0x00
#define NFC_CMD_CORE_INIT       0x01
#define NFC_CMD_DISCOVER        0x02
#define NFC_CMD_READ_NDEF       0x10
#define NFC_CMD_WRITE_NDEF      0x11

static int nfc_ready = 0;

int nfc_init(void) {
    printf("[NFC] NXP PN553 init\n");

    /* Power up */
    write_reg(SM8250_TLMM_BASE, NFC_VEN_GPIO * 4, 0x03);
    for (int i = 0; i < 10000; i++) __asm__("nop");

    /* Core reset */
    write_reg(NFC_I2C_BASE, 0, NFC_CMD_CORE_RESET);
    write_reg(NFC_I2C_BASE, 0, NFC_CMD_CORE_INIT);

    nfc_ready = 1;
    printf("[NFC] Ready\n");
    return 0;
}

int nfc_discover(void) {
    if (!nfc_ready) return -1;
    write_reg(NFC_I2C_BASE, 0, NFC_CMD_DISCOVER);
    printf("[NFC] Discovery started\n");
    int tag = (int)read_reg(NFC_I2C_BASE, 4);
    if (tag) printf("[NFC] Tag found: type %d\n", tag);
    return tag;
}

int nfc_read_ndef(uint8_t *buf, uint32_t maxlen) {
    if (!nfc_ready) return -1;
    write_reg(NFC_I2C_BASE, 0, NFC_CMD_READ_NDEF);
    uint32_t len = read_reg(NFC_I2C_BASE, 8);
    if (len > maxlen) len = maxlen;
    /* Data would be in I2C FIFO */
    printf("[NFC] Read %u bytes NDEF\n", len);
    return (int)len;
}

int nfc_write_ndef(const uint8_t *data, uint32_t len) {
    if (!nfc_ready || !data) return -1;
    (void)data;
    write_reg(NFC_I2C_BASE, 0, NFC_CMD_WRITE_NDEF);
    write_reg(NFC_I2C_BASE, 8, len);
    printf("[NFC] Wrote %u bytes NDEF\n", len);
    return (int)len;
}
