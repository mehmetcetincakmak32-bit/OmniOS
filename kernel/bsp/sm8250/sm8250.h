/* OmniOS — kernel/bsp/sm8250/sm8250.h */
/* Snapdragon 865 (SM8250) Board Support Package */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_SM8250_H
#define OMNIOS_SM8250_H

#include <stdint.h>
#include <stdbool.h>

/* ── SoC memory map ──────────────────────────────────────────────── */
#define SM8250_UART_BASE     0x0A84000
#define SM8250_PMIC_BASE     0x0AF0000  /* PM8998/PM8150 */
#define SM8250_TLMM_BASE     0x0F00000  /* GPIO */
#define SM8250_QUPV3_BASE    0x0C00000  /* I2C/SPI/UART */
#define SM8250_UFS_BASE      0x01D84000
#define SM8250_USB3_BASE     0x0A6F8800
#define SM8250_MSS_BASE      0x04080000  /* Modem DSP */
#define SM8250_ADSP_BASE     0x03000000  /* Audio DSP */
#define SM8250_DISPLAY_BASE  0x0AE00000  /* MDSS + DSI */
#define SM8250_GPU_BASE      0x03D00000  /* Adreno 650 */
#define SM8250_CAMERA_BASE   0x0ACB0000  /* CAMSS */
#define SM8250_SENSOR_BASE   0x0C2F0000  /* LPASS + SLPI */

/* ── PMIC (PM8150) registers ─────────────────────────────────────── */
#define PM8150_REVISION      0x0000
#define PM8150_VREG_CTRL     0x4100
#define PM8150_VREG_STATUS   0x4108
#define PM8150_CHG_CTRL      0x4500
#define PM8150_CHG_STATUS    0x4508
#define PM8150_FG_ACCUM      0x4700   /* Fuel gauge accumulator */
#define PM8150_FG_VOLTAGE    0x4708
#define PM8150_FG_TEMP       0x4710
#define PM8150_FG_SOC        0x4718   /* State of charge % */
#define PM8150_TEMP_ALARM    0x4900

/* ── Display (MDSS + DSI) registers ──────────────────────────────── */
#define MDSS_CTRL            0x00000
#define MDSS_INTF_CTRL       0x00100
#define MDSS_DSI0_CTRL       0x10000
#define MDSS_DSI0_PHY        0x10080
#define MDSS_DSI1_CTRL       0x12000
#define MDSS_HDMI_CTRL       0x20000
#define MDSS_MDP_CTRL        0x01000
#define MDSS_MDP_LAYER(n)    (0x01100 + (n) * 0x100)

/* ── Touch (I2C) config ──────────────────────────────────────────── */
#define TOUCH_I2C_BUS        0x88000  /* QUPv3 I2C bus 0 */
#define TOUCH_INT_GPIO       96
#define TOUCH_RST_GPIO       97

/* ── Modem (MSS) registers ────────────────────────────────────────── */
#define MSS_CTRL             0x00000
#define MSS_STATUS           0x00004
#define MSS_PIL_ADDR         0x01000  /* Modem firmware load addr */
#define MSS_PIL_SIZE         0x200000

/* ── Sensors (SLPI) ──────────────────────────────────────────────── */
#define SLPI_CTRL            0x00000
#define SLPI_STATUS          0x00008
#define SLPI_ACCEL_DATA      0x00100
#define SLPI_GYRO_DATA       0x00108
#define SLPI_MAG_DATA        0x00110
#define SLPI_LIGHT_DATA      0x00118
#define SLPI_PROX_DATA       0x00120
#define SLPI_PRESSURE_DATA   0x00128

/* ── Public API ──────────────────────────────────────────────────── */
int  sm8250_init(void);
void sm8250_poweroff(void);
void sm8250_reboot(void);

/* PMIC */
int  pm8150_init(void);
int  pm8150_charger_status(void);
int  pm8150_battery_soc(void);
int  pm8150_battery_voltage(void);
int  pm8150_battery_temp(void);
void pm8150_charger_enable(bool on);

/* Display */
int  mdss_init(void);
void mdss_panel_on(void);
void mdss_panel_off(void);
void mdss_set_brightness(uint32_t level);

/* Touch */
int  touch_init(void);
void touch_power_on(void);
void touch_power_off(void);

/* Modem */
int  mss_init(void);
int  mss_load_firmware(const void *fw, size_t size);
int  mss_start(void);

/* Sensors */
int  slpi_init(void);
void slpi_read_accel(float *x, float *y, float *z);
void slpi_read_gyro(float *x, float *y, float *z);
int  slpi_read_light(void);
int  slpi_read_proximity(void);

#endif /* OMNIOS_SM8250_H */
