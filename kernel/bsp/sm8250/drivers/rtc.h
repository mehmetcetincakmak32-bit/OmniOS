/* OmniOS — kernel/bsp/sm8250/drivers/rtc.h */
/* RTC interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_RTC_H
#define OMNIOS_RTC_H

#include <stdint.h>

typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
} rtc_time_t;

int      rtc_init(void);
uint64_t rtc_get_epoch(void);
void     rtc_set_epoch(uint64_t epoch);
void     rtc_get_time(rtc_time_t *t);
void     rtc_set_time(const rtc_time_t *t);
void     rtc_set_alarm(uint64_t epoch);

#endif
