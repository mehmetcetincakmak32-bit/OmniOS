/* OmniOS — kernel/bsp/sm8250/drivers/power_mgmt.c */
/* Mobile power management: suspend, cpufreq, thermal, battery */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <stdbool.h>

#define LPM_BASE          0x0B00000  /* Low power manager */
#define CPR_BASE          0x0C00000  /* Core power rail */
#define THERMAL_BASE      0x0D00000
#define CPUFREQ_BASE      0x0E00000

static bool suspended = false;

int mobile_power_init(void) {
    printf("[POWER] Mobile PM init\n");

    /* Config LPM */
    write_reg(LPM_BASE, 0x00, 0x07);    /* Enable all low-power modes */
    write_reg(LPM_BASE, 0x04, 0x01);    /* Retention on */

    /* CPU freq range */
    write_reg(CPUFREQ_BASE, 0x00, 300000);   /* 300 MHz min */
    write_reg(CPUFREQ_BASE, 0x04, 2840000);  /* 2.84 GHz max */
    write_reg(CPUFREQ_BASE, 0x08, 825000);   /* 825 mV */

    /* Thermal config */
    write_reg(THERMAL_BASE, 0x00, 70000);    /* 70°C throttle */
    write_reg(THERMAL_BASE, 0x04, 85000);    /* 85°C shutdown */

    printf("[POWER] Battery: %d%%\n", pm8150_battery_soc());
    return 0;
}

void mobile_suspend(void) {
    if (suspended) return;
    printf("[POWER] Entering suspend\n");
    mdss_panel_off();
    pm8150_charger_enable(false);
    write_reg(LPM_BASE, 0x10, 1);
    suspended = true;
}

void mobile_resume(void) {
    if (!suspended) return;
    printf("[POWER] Resuming\n");
    write_reg(LPM_BASE, 0x10, 0);
    pm8150_charger_enable(true);
    mdss_panel_on();
    mdss_set_brightness(128);
    suspended = false;
}

int mobile_set_cpufreq(uint32_t khz) {
    if (khz < 300000) khz = 300000;
    if (khz > 2840000) khz = 2840000;
    write_reg(CPUFREQ_BASE, 0x100, khz);
    return 0;
}

int mobile_get_temp(void) {
    return (int)read_reg(THERMAL_BASE, 0x100);
}

void mobile_cool_down(void) {
    int temp = mobile_get_temp();
    if (temp > 70000) {
        mobile_set_cpufreq(300000);
        mdss_set_brightness(64);
        printf("[POWER] Thermal throttle: %d°C\n", temp);
    }
}
