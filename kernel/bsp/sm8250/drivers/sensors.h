/* OmniOS — kernel/bsp/sm8250/drivers/sensors.h */
/* Mobile sensor interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_MOBILE_SENSORS_H
#define OMNIOS_MOBILE_SENSORS_H

int mobile_sensors_init(void);
int mobile_sensors_read_accel(float *x, float *y, float *z);
int mobile_sensors_read_gyro(float *x, float *y, float *z);
int mobile_sensors_read_light(void);
int mobile_sensors_read_proximity(void);
int mobile_sensors_is_covered(void);
int mobile_sensors_orientation(void);
void mobile_sensors_set_thresholds(int prox_cm, int light_lux);

#endif
