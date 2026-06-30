/* OmniOS — kernel/bsp/sm8250/drivers/usb_gadget.h */
/* USB gadget — ADB + MTP interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_USB_GADGET_H
#define OMNIOS_USB_GADGET_H

#include <stdint.h>

int  usb_gadget_init(void);
int  usb_gadget_connected(void);

int  adb_send(const uint8_t *data, uint32_t len);
int  adb_recv(uint8_t *buf, uint32_t maxlen);
void adb_shell(const char *cmd);

int  mtp_send_object(const char *name, const uint8_t *data, uint32_t size);
int  mtp_recv_object(char *name, uint32_t namelen, uint8_t *buf, uint32_t maxlen);

#endif
