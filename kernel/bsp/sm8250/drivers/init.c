/* OmniOS — kernel/bsp/sm8250/drivers/init.c */
/* Mobile init process — hardware init + userspace launch */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include "display_fb.h"
#include "input.h"
#include "audio.h"
#include "ufs.h"
#include "usb_gadget.h"
#include "rtc.h"
#include "nfc.h"
#include "fingerprint.h"
#include "power_mgmt.h"
#include "sensors.h"
#include "modem_ril.h"
#include "wifi_bt.h"
#include <stdio.h>

static int init_done = 0;

void mobile_boot_init(void) {
    printf("\n========================================\n");
    printf("  OmniOS ARM64 Mobile — Boot Init\n");
    printf("========================================\n");

    /* Core hardware */
    ufs_init();
    display_fb_init();
    input_init();
    audio_init();

    /* Connectivity */
    wifi_init();
    bt_init();
    gps_init();
    nfc_init();

    /* Telecom */
    ril_init();
    ims_init();

    /* Security */
    fingerprint_init();

    /* Time */
    rtc_init();

    /* USB */
    usb_gadget_init();
    usb_otg_init();

    /* Power management */
    mobile_power_init();

    /* Display ON */
    mdss_panel_on();
    mdss_set_brightness(128);

    /* Status */
    printf("\n========================================\n");
    printf("  OmniOS v1.0 ARM64 Mobile\n");
    printf("  Battery: %d%% (%dmV, %d°C) %s\n",
        pm8150_battery_soc(), pm8150_battery_voltage(),
        pm8150_battery_temp(),
        pm8150_charger_status() ? "CHARGING" : "DISCHARGING");
    printf("  Signal: %d dBm  IMS: %s\n",
        ril_get_signal(),
        ims_is_registered() ? "REGISTERED" : "OFFLINE");
    printf("  Wi-Fi: SCANNING  BT: READY  NFC: READY\n");
    printf("  FP: %d enrolled  USB: %s\n",
        fingerprint_count(),
        usb_gadget_connected() ? "CONNECTED" : "DISCONNECTED");
    printf("  RTC: %llu epoch\n", (unsigned long long)rtc_get_epoch());
    printf("  Display: 1080x2340 @ 60Hz\n");
    printf("  Storage: %llu MB\n",
        (unsigned long long)(ufs_get_capacity() / 1024 / 1024));
    printf("========================================\n\n");

    init_done = 1;
}

int mobile_boot_is_complete(void) {
    return init_done;
}

void mobile_boot_animation(void) {
    uint32_t w, h, bpp;
    uintptr_t addr;
    if (display_fb_get_info(&w, &h, &bpp, &addr) < 0) return;

    /* Draw OmniOS boot logo (white text on dark background) */
    display_fb_fill_rect(0, 0, w, h, 0x000000);

    /* Top status bar */
    display_fb_fill_rect(0, 0, w, 48, 0x1A1A2E);

    /* Center logo area */
    for (int i = 0; i < 64; i++) {
        display_fb_fill_rect(w/2 - 64, h/2 - 32 + i, 128, 1, 0x00BCD4);
    }

    /* Bottom bar */
    display_fb_fill_rect(0, h - 48, w, 48, 0x1A1A2E);

    display_fb_flush();
}
