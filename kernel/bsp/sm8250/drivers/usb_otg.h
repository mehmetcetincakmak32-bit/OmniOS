/* OmniOS — kernel/bsp/sm8250/drivers/usb_otg.h */
/* USB Type-C OTG + charging interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_USB_OTG_H
#define OMNIOS_USB_OTG_H

int usb_otg_init(void);
int usb_otg_mode_host(void);
int usb_otg_mode_device(void);
int usb_otg_charging_current(void);

#endif
