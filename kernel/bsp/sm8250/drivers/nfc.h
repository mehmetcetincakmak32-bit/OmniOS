/* OmniOS — kernel/bsp/sm8250/drivers/nfc.h */
/* NFC interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_NFC_H
#define OMNIOS_NFC_H

#include <stdint.h>

int  nfc_init(void);
int  nfc_discover(void);
int  nfc_read_ndef(uint8_t *buf, uint32_t maxlen);
int  nfc_write_ndef(const uint8_t *data, uint32_t len);

#endif
