/* OmniOS — kernel/bsp/sm8250/drivers/power_mgmt.h */
/* Mobile power management interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_MOBILE_POWER_H
#define OMNIOS_MOBILE_POWER_H

#include <stdint.h>
#include <stdbool.h>

int  mobile_power_init(void);
void mobile_suspend(void);
void mobile_resume(void);
int  mobile_set_cpufreq(uint32_t khz);
int  mobile_get_temp(void);
void mobile_cool_down(void);

#endif
