/* OmniOS — kernel/bsp/sm8250/drivers/usb_otg.c */
/* USB 3.0 Type-C (DWC3) OTG + charging */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>

#define DWC3_BASE        0x0A600000
#define DWC3_GCTL        0x0C110  /* Global control */
#define DWC3_DCTL        0x0C114  /* Device control */
#define DWC3_GSTS        0x0C118  /* Global status */

int usb_otg_init(void) {
    printf("[USB] DWC3 USB 3.0 Type-C init\n");

    /* Reset core */
    write_reg(DWC3_BASE, DWC3_GCTL, 0x02);
    uint32_t sts;
    for (int i = 0; i < 1000; i++) {
        sts = read_reg(DWC3_BASE, DWC3_GSTS);
        if (!(sts & 2)) break;
    }

    /* Set as peripheral (device mode) */
    write_reg(DWC3_BASE, DWC3_GCTL, 0x01);

    printf("[USB] Ready\n");
    return 0;
}

int usb_otg_mode_host(void) {
    write_reg(DWC3_BASE, DWC3_GCTL, 0x03);
    printf("[USB] Host mode\n");
    return 0;
}

int usb_otg_mode_device(void) {
    write_reg(DWC3_BASE, DWC3_GCTL, 0x01);
    printf("[USB] Device mode\n");
    return 0;
}

int usb_otg_charging_current(void) {
    uint32_t sts = read_reg(DWC3_BASE, DWC3_GSTS);
    if (sts & 0x100) return 3000;  /* 3A PD */
    if (sts & 0x080) return 1500;  /* 1.5A BC1.2 */
    return 500;                     /* 500mA default */
}
