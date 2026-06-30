/* OmniOS — kernel/bsp/sm8250/drivers/fingerprint.h */
/* Fingerprint sensor interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_FINGERPRINT_H
#define OMNIOS_FINGERPRINT_H

int fingerprint_init(void);
int fingerprint_enroll(const char *name);
int fingerprint_authenticate(void);
int fingerprint_delete(int id);
int fingerprint_count(void);

#endif
