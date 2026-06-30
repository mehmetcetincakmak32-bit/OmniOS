/* OmniOS — kernel/bsp/sm8250/drivers/rtc.c */
/* PM8150 RTC — real-time clock */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <stdint.h>

#define RTC_BASE          (SM8250_PMIC_BASE + 0x6000)
#define RTC_TIME_LOW      0x00
#define RTC_TIME_HIGH     0x04
#define RTC_ALARM_LOW     0x08
#define RTC_ALARM_HIGH    0x0C
#define RTC_CTRL          0x10
#define RTC_STATUS        0x14

typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
} rtc_time_t;

static int rtc_initialized = 0;
static uint64_t rtc_offset = 1700000000;  /* Unix epoch as base */

static int is_leap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

static int days_in_month(int y, int m) {
    static const int d[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && is_leap(y)) return 29;
    return d[m - 1];
}

static uint64_t time_to_epoch(const rtc_time_t *t) {
    int y = t->year;
    int m = t->month;
    uint64_t days = (y - 1970) * 365 + (y - 1969) / 4;
    for (int i = 1; i < m; i++) days += days_in_month(y, i);
    days += t->day - 1;
    return days * 86400 + t->hour * 3600 + t->minute * 60 + t->second;
}

static void epoch_to_time(uint64_t epoch, rtc_time_t *t) {
    int y = 1970;
    while (1) {
        int d = is_leap(y) ? 366 : 365;
        if (epoch < (uint64_t)d * 86400) break;
        epoch -= (uint64_t)d * 86400;
        y++;
    }
    t->year = y;
    for (int m = 1; m <= 12; m++) {
        int d = days_in_month(y, m);
        if (epoch < (uint64_t)d * 86400) { t->month = m; break; }
        epoch -= (uint64_t)d * 86400;
    }
    t->day = (uint8_t)(epoch / 86400) + 1;
    epoch %= 86400;
    t->hour   = (uint8_t)(epoch / 3600);
    epoch %= 3600;
    t->minute = (uint8_t)(epoch / 60);
    t->second = (uint8_t)(epoch % 60);
}

int rtc_init(void) {
    printf("[RTC] PM8150 RTC init\n");
    write_reg(RTC_BASE, RTC_CTRL, 1);

    /* Try reading hardware time */
    uint32_t low  = read_reg(RTC_BASE, RTC_TIME_LOW);
    uint32_t high = read_reg(RTC_BASE, RTC_TIME_HIGH);
    if (low || high) {
        rtc_offset = ((uint64_t)high << 32) | low;
    }

    rtc_initialized = 1;
    rtc_time_t t;
    epoch_to_time(rtc_offset, &t);
    printf("[RTC] %04d-%02d-%02d %02d:%02d:%02d\n",
        t.year, t.month, t.day, t.hour, t.minute, t.second);
    return 0;
}

uint64_t rtc_get_epoch(void) {
    if (!rtc_initialized) return 0;
    uint32_t low  = read_reg(RTC_BASE, RTC_TIME_LOW);
    uint32_t high = read_reg(RTC_BASE, RTC_TIME_HIGH);
    return ((uint64_t)high << 32) | low;
}

void rtc_set_epoch(uint64_t epoch) {
    rtc_offset = epoch;
    write_reg(RTC_BASE, RTC_TIME_LOW,  (uint32_t)(epoch & 0xFFFFFFFF));
    write_reg(RTC_BASE, RTC_TIME_HIGH, (uint32_t)(epoch >> 32));
}

void rtc_get_time(rtc_time_t *t) {
    epoch_to_time(rtc_get_epoch(), t);
}

void rtc_set_time(const rtc_time_t *t) {
    rtc_set_epoch(time_to_epoch(t));
}

void rtc_set_alarm(uint64_t epoch) {
    write_reg(RTC_BASE, RTC_ALARM_LOW,  (uint32_t)(epoch & 0xFFFFFFFF));
    write_reg(RTC_BASE, RTC_ALARM_HIGH, (uint32_t)(epoch >> 32));
    printf("[RTC] Alarm set\n");
}
