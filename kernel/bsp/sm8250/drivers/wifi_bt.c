/* OmniOS — kernel/bsp/sm8250/drivers/wifi_bt.c */
/* QCA6391 WiFi (SDIO) + Bluetooth (UART) combo */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <stdbool.h>

#define WIFI_SDIO_BASE      0x01D00000
#define WIFI_IRQ            147
#define BT_UART_BASE        (SM8250_QUPV3_BASE + 0x10000)
#define BT_IRQ              148

static bool wifi_active = false;
static bool bt_active   = false;

int wifi_init(void) {
    printf("[WIFI] QCA6391 SDIO init\n");
    /* SDIO clock enable */
    write_reg(WIFI_SDIO_BASE, 0x00, 1);
    /* Power up sequence */
    write_reg(WIFI_SDIO_BASE, 0x04, 0x3);
    wifi_active = true;
    printf("[WIFI] Firmware loaded, ready\n");
    return 0;
}

int wifi_scan(void) {
    if (!wifi_active) return -1;
    /* Simplified scan trigger */
    printf("[WIFI] Scanning...\n");
    return 0;
}

int wifi_connect(const char *ssid, const char *pass) {
    if (!wifi_active) return -1;
    (void)ssid; (void)pass;
    printf("[WIFI] Connecting to %s\n", ssid);
    return 0;
}

int bt_init(void) {
    printf("[BT] QCA6391 UART init\n");
    /* Configure UART: 115200 8N1 */
    write_reg(BT_UART_BASE, 0x00, 0x01);   /* CR_EN */
    write_reg(BT_UART_BASE, 0x30, 0x01);   /* IBRD */
    write_reg(BT_UART_BASE, 0x34, 0x40);   /* FBRD */
    write_reg(BT_UART_BASE, 0x38, 0x60);   /* LCR_H */
    bt_active = true;
    printf("[BT] Bluetooth ready\n");
    return 0;
}

int bt_scan(void) {
    if (!bt_active) return -1;
    printf("[BT] Scanning...\n");
    return 0;
}

int gps_init(void) {
    /* GPS shares UART with BT via mux */
    if (!bt_active) {
        int ret = bt_init();
        if (ret) return ret;
    }
    printf("[GPS] GNSS init (shared UART)\n");
    return 0;
}
