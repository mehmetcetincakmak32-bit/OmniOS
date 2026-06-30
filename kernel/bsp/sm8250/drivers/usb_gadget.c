/* OmniOS — kernel/bsp/sm8250/drivers/usb_gadget.c */
/* USB gadget — ADB (Android Debug Bridge) + MTP */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <string.h>

#define USB_GADGET_BASE    (SM8250_USB3_BASE + 0x10000)
#define GADGET_CTRL        0x000
#define GADGET_STATUS      0x004
#define GADGET_EP_BASE     0x100
#define GADGET_FIFO_BASE   0x1000

#define EP_OUT             0x01
#define EP_IN              0x81

static uint8_t _adb_buffer[4096];
static uint8_t _mtp_buffer[65536];
static int _connected = 0;

int usb_gadget_init(void) {
    printf("[USB_GADGET] USB 3.0 gadget init\n");

    /* Reset gadget core */
    write_reg(USB_GADGET_BASE, GADGET_CTRL, 0x02);
    for (int i = 0; i < 100000; i++) {
        if (!(read_reg(USB_GADGET_BASE, GADGET_STATUS) & 0x02)) break;
    }

    /* Configure endpoints: EP0 control, EP1 bulk out, EP2 bulk in */
    write_reg(USB_GADGET_BASE, GADGET_EP_BASE + 0x00, 0x80);  /* EP0: 64 bytes */
    write_reg(USB_GADGET_BASE, GADGET_EP_BASE + 0x04, 0x02);  /* EP1: bulk out, 512 */
    write_reg(USB_GADGET_BASE, GADGET_EP_BASE + 0x08, 0x82);  /* EP2: bulk in, 512 */

    /* Enable gadget */
    write_reg(USB_GADGET_BASE, GADGET_CTRL, 0x01);

    printf("[USB_GADGET] Ready: ADB + MTP\n");
    return 0;
}

int usb_gadget_connected(void) {
    return read_reg(USB_GADGET_BASE, GADGET_STATUS) & 0x01;
}

/* ── ADB ─────────────────────────────────────────────────────────── */

int adb_send(const uint8_t *data, uint32_t len) {
    if (len > sizeof(_adb_buffer)) len = sizeof(_adb_buffer);
    memcpy((void *)(USB_GADGET_BASE + GADGET_FIFO_BASE), data, len);
    write_reg(USB_GADGET_BASE, GADGET_EP_BASE + 0x08, len | 0x80000000);
    return (int)len;
}

int adb_recv(uint8_t *buf, uint32_t maxlen) {
    uint32_t avail = read_reg(USB_GADGET_BASE, GADGET_EP_BASE + 0x04);
    if (!avail) return 0;
    if (avail > maxlen) avail = maxlen;
    memcpy(buf, (const void *)(USB_GADGET_BASE + GADGET_FIFO_BASE + 0x1000), avail);
    return (int)avail;
}

void adb_shell(const char *cmd) {
    printf("[ADB] shell: %s\n", cmd);
}

/* ── MTP ─────────────────────────────────────────────────────────── */

int mtp_send_object(const char *name, const uint8_t *data, uint32_t size) {
    (void)name;
    if (size > sizeof(_mtp_buffer)) return -1;
    memcpy((void *)(USB_GADGET_BASE + GADGET_FIFO_BASE + 0x2000), data, size);
    printf("[MTP] Sent %s (%u bytes)\n", name, size);
    return (int)size;
}

int mtp_recv_object(char *name, uint32_t namelen, uint8_t *buf, uint32_t maxlen) {
    uint32_t avail = read_reg(USB_GADGET_BASE, GADGET_EP_BASE + 0x04);
    if (!avail) return 0;
    if (avail > maxlen) avail = maxlen;
    memcpy(buf, (const void *)(USB_GADGET_BASE + GADGET_FIFO_BASE + 0x2000), avail);
    snprintf(name, namelen, "mtp_object_%u", avail);
    return (int)avail;
}
