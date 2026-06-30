/* OmniOS — kernel/bsp/sm8250/sm8250.c */
/* Snapdragon 865 BSP init */
/* SPDX-License-Identifier: MIT */

#include "sm8250.h"
#include <stdio.h>
#include <string.h>

static inline uint32_t read_reg(uintptr_t base, uint32_t off) {
    return *(volatile uint32_t *)(base + off);
}
static inline void write_reg(uintptr_t base, uint32_t off, uint32_t val) {
    *(volatile uint32_t *)(base + off) = val;
}

int pm8150_init(void) {
    printf("[PMIC] PM8150 rev 0x%x\n", read_reg(SM8250_PMIC_BASE, PM8150_REVISION));
    write_reg(SM8250_PMIC_BASE, PM8150_CHG_CTRL, 1);
    return 0;
}

int pm8150_charger_status(void) {
    return read_reg(SM8250_PMIC_BASE, PM8150_CHG_STATUS) & 1;
}

int pm8150_battery_soc(void) {
    return read_reg(SM8250_PMIC_BASE, PM8150_FG_SOC) & 0xFF;
}

int pm8150_battery_voltage(void) {
    return read_reg(SM8250_PMIC_BASE, PM8150_FG_VOLTAGE);
}

int pm8150_battery_temp(void) {
    return (int)(int8_t)(read_reg(SM8250_PMIC_BASE, PM8150_FG_TEMP) & 0xFF);
}

void pm8150_charger_enable(bool on) {
    write_reg(SM8250_PMIC_BASE, PM8150_CHG_CTRL, on ? 1 : 0);
}

int mdss_init(void) {
    printf("[DISPLAY] MDSS + DSI init\n");
    write_reg(SM8250_DISPLAY_BASE, MDSS_DSI0_CTRL, 0);
    write_reg(SM8250_DISPLAY_BASE, MDSS_DSI0_PHY, 0x3FF);
    write_reg(SM8250_DISPLAY_BASE, MDSS_MDP_CTRL, 1);
    write_reg(SM8250_DISPLAY_BASE, MDSS_INTF_CTRL, (1080 << 16) | 2340);
    return 0;
}

void mdss_panel_on(void) {
    write_reg(SM8250_DISPLAY_BASE, MDSS_CTRL, 1);
    printf("[DISPLAY] Panel ON\n");
}

void mdss_panel_off(void) {
    write_reg(SM8250_DISPLAY_BASE, MDSS_CTRL, 0);
    printf("[DISPLAY] Panel OFF\n");
}

void mdss_set_brightness(uint32_t level) {
    if (level > 255) level = 255;
    write_reg(SM8250_PMIC_BASE + 0x5000, 0, level);
}

int touch_init(void) {
    write_reg(SM8250_TLMM_BASE, TOUCH_RST_GPIO * 4, 0x03);
    write_reg(SM8250_TLMM_BASE, TOUCH_INT_GPIO * 4, 0x00);
    write_reg(SM8250_TLMM_BASE, TOUCH_RST_GPIO * 4, 0);
    printf("[TOUCH] Synaptics S3908 init\n");
    return 0;
}

void touch_power_on(void) { write_reg(SM8250_TLMM_BASE, TOUCH_RST_GPIO * 4, 0x03); }
void touch_power_off(void) { write_reg(SM8250_TLMM_BASE, TOUCH_RST_GPIO * 4, 0); }

int mss_init(void) {
    printf("[MODEM] X55 modem init\n");
    write_reg(SM8250_MSS_BASE, MSS_CTRL, 1);
    printf("[MODEM] Status: 0x%x\n", read_reg(SM8250_MSS_BASE, MSS_STATUS));
    return 0;
}

int mss_load_firmware(const void *fw, size_t size) {
    if (size > MSS_PIL_SIZE) return -1;
    memcpy((void *)(SM8250_MSS_BASE + MSS_PIL_ADDR), fw, size);
    printf("[MODEM] Firmware loaded: %zu bytes\n", size);
    return 0;
}

int mss_start(void) {
    write_reg(SM8250_MSS_BASE, MSS_CTRL, 3);
    printf("[MODEM] Modem started\n");
    return 0;
}

int slpi_init(void) {
    printf("[SENSORS] SLPI DSP\n");
    write_reg(SM8250_SENSOR_BASE, SLPI_CTRL, 1);
    return 0;
}

void slpi_read_accel(float *x, float *y, float *z) {
    uint32_t raw = read_reg(SM8250_SENSOR_BASE, SLPI_ACCEL_DATA);
    *x = (float)(int16_t)(raw & 0xFFFF) / 16384.0f;
    *y = (float)(int16_t)((raw >> 16) & 0xFFFF) / 16384.0f;
    *z = 9.81f;
}

void slpi_read_gyro(float *x, float *y, float *z) {
    uint32_t raw = read_reg(SM8250_SENSOR_BASE, SLPI_GYRO_DATA);
    *x = (float)(int16_t)(raw & 0xFFFF) / 131.0f;
    *y = (float)(int16_t)((raw >> 16) & 0xFFFF) / 131.0f;
    *z = 0;
}

int slpi_read_light(void) {
    return (int)(read_reg(SM8250_SENSOR_BASE, SLPI_LIGHT_DATA) & 0xFFFF);
}

int slpi_read_proximity(void) {
    return (int)(read_reg(SM8250_SENSOR_BASE, SLPI_PROX_DATA) & 0xFFFF);
}

int sm8250_init(void) {
    printf("[SM8250] Snapdragon 865 BSP init\n");
    pm8150_init();
    mdss_init();
    touch_init();
    mss_init();
    slpi_init();

    printf("[BSP] Battery: %d%% (%dmV, %d°C) %s\n",
        pm8150_battery_soc(), pm8150_battery_voltage(),
        pm8150_battery_temp(),
        pm8150_charger_status() ? "CHARGING" : "DISCHARGING");
    printf("[BSP] Light: %d lux  Proximity: %d cm\n",
        slpi_read_light(), slpi_read_proximity());

    mdss_panel_on();
    mdss_set_brightness(128);
    printf("[SM8250] BSP ready\n");
    return 0;
}

void sm8250_poweroff(void) {
    mdss_panel_off();
    pm8150_charger_enable(false);
    write_reg(SM8250_PMIC_BASE, 0x4F00, 0x10);
    while (1) __asm__("wfi");
}

void sm8250_reboot(void) {
    write_reg(SM8250_PMIC_BASE, 0x4F00, 0x20);
    while (1) __asm__("wfi");
}
