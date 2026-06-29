#ifndef OMNOS_SENSOR_HAL_H
#define OMNOS_SENSOR_HAL_H

#include "../include/omnios_kernel.h"

typedef enum {
    SENSOR_TYPE_ACCELEROMETER = 0,
    SENSOR_TYPE_GYROSCOPE = 1,
    SENSOR_TYPE_MAGNETOMETER = 2,
    SENSOR_TYPE_LIGHT = 3,
    SENSOR_TYPE_PROXIMITY = 4,
    SENSOR_TYPE_PRESSURE = 5,
    SENSOR_TYPE_HUMIDITY = 6,
    SENSOR_TYPE_TEMPERATURE = 7,
    SENSOR_TYPE_STEP_COUNTER = 8,
    SENSOR_TYPE_HEART_RATE = 9,
    SENSOR_TYPE_FINGERPRINT = 10,
    SENSOR_TYPE_COUNT
} sensor_type_t;

typedef struct {
    float x;
    float y;
    float z;
    float accuracy;
    uint64_t timestamp;
} vector3_t;

typedef struct {
    float value;
    float range;
    float accuracy;
    uint64_t timestamp;
} scalar_t;

typedef struct {
    sensor_type_t type;
    union {
        vector3_t vector;
        scalar_t scalar;
        char text[32];
    } data;
    bool available;
    uint32_t sampling_rate;
} sensor_event_t;

typedef void (*sensor_callback_t)(sensor_type_t type, const sensor_event_t* event);

status_t sensor_hal_init(void);
void     sensor_hal_shutdown(void);
status_t sensor_hal_enable(sensor_type_t type, uint32_t sampling_rate);
status_t sensor_hal_disable(sensor_type_t type);
bool     sensor_hal_is_available(sensor_type_t type);
sensor_event_t* sensor_hal_get_last_event(sensor_type_t type);
status_t sensor_hal_register_callback(sensor_callback_t cb);

#endif