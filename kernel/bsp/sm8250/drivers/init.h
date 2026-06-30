/* OmniOS — kernel/bsp/sm8250/drivers/init.h */
/* Mobile boot init interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_MOBILE_INIT_H
#define OMNIOS_MOBILE_INIT_H

#include <stdbool.h>

void mobile_boot_init(void);
int  mobile_boot_is_complete(void);
void mobile_boot_animation(void);

#endif
