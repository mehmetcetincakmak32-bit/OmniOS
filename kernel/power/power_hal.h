#ifndef OMNOS_POWER_HAL_H
#define OMNOS_POWER_HAL_H

#include "../include/omnios_kernel.h"

typedef struct {
    uint32_t level;        /* 0-100 percentage */
    uint32_t voltage_mv;   /* millivolts */
    uint32_t capacity_mah; /* milliamp-hours */
    uint64_t timestamp;     /* last update time */
    bool charging;         /* currently charging */
    bool plugged;          /* power source connected */
    bool usb_present;      /* USB power connected */
    bool wireless_present; /* wireless charging connected */
    uint32_t temperature_c; /* degrees Celsius */
    uint32_t health;        /* 1=good, 2=degraded, 3=bad */
} battery_info_t;

typedef struct {
    uint32_t level;        /* 0-100 percentage */
    int32_t  temperature_c; /* Celsius */
    uint32_t cpu_freq;     /* MHz */
    uint32_t screen_time_ms; /* time screen active */
} power_stats_t;

status_t power_hal_init(void);
void     power_hal_shutdown(void);
battery_info_t* power_hal_get_battery_info(void);
status_t power_hal_set_charge_enable(bool enable);
bool     power_hal_is_charging(void);
uint32_t power_hal_get_level(void);
power_stats_t* power_hal_get_stats(void);
status_t power_hal_enter_suspend(void);
status_t power_hal_exit_suspend(void);

#endif