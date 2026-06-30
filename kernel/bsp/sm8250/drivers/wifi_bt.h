/* OmniOS — kernel/bsp/sm8250/drivers/wifi_bt.h */
/* QCA6391 WiFi (SDIO) + Bluetooth (UART) interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_WIFI_BT_H
#define OMNIOS_WIFI_BT_H

int wifi_init(void);
int wifi_scan(void);
int wifi_connect(const char *ssid, const char *pass);
int bt_init(void);
int bt_scan(void);
int gps_init(void);

#endif
