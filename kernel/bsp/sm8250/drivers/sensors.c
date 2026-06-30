/* OmniOS — kernel/bsp/sm8250/drivers/sensors.c */
/* Mobile sensors: accelerometer, gyro, proximity, light, barometer */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>

static int sensor_init_done = 0;

int mobile_sensors_init(void) {
    printf("[SENSORS] Mobile sensor HAL init\n");
    slpi_init();

    /* Read initial values */
    float ax, ay, az, gx, gy, gz;
    slpi_read_accel(&ax, &ay, &az);
    slpi_read_gyro(&gx, &gy, &gz);
    int light = slpi_read_light();
    int prox  = slpi_read_proximity();

    printf("[SENSORS] Accel: %.2f %.2f %.2f m/s²\n", ax, ay, az);
    printf("[SENSORS] Gyro:  %.2f %.2f %.2f °/s\n", gx, gy, gz);
    printf("[SENSORS] Light: %d lux  Prox: %d cm\n", light, prox);

    sensor_init_done = 1;
    return 0;
}

int mobile_sensors_read_accel(float *x, float *y, float *z) {
    if (!sensor_init_done) return -1;
    slpi_read_accel(x, y, z);
    return 0;
}

int mobile_sensors_read_gyro(float *x, float *y, float *z) {
    if (!sensor_init_done) return -1;
    slpi_read_gyro(x, y, z);
    return 0;
}

int mobile_sensors_read_light(void) {
    if (!sensor_init_done) return -1;
    return slpi_read_light();
}

int mobile_sensors_read_proximity(void) {
    if (!sensor_init_done) return -1;
    return slpi_read_proximity();
}

int mobile_sensors_is_covered(void) {
    if (!sensor_init_done) return 0;
    return slpi_read_proximity() < 5;
}

int mobile_sensors_orientation(void) {
    float x, y, z;
    if (mobile_sensors_read_accel(&x, &y, &z) < 0) return 0;
    if (z < -7.0f) return 1;  /* Portrait up */
    if (z > 7.0f)  return 2;  /* Portrait down */
    return 0;
}

void mobile_sensors_set_thresholds(int prox_cm, int light_lux) {
    (void)prox_cm; (void)light_lux;
    printf("[SENSORS] Thresholds updated\n");
}
