#include "sensor_hal.h"
#include <stdio.h>
#include <string.h>

static sensor_callback_t _sensor_callback = NULL;
static sensor_event_t _last_events[SENSOR_TYPE_COUNT];

typedef struct {
    sensor_type_t type;
    char name[32];
    bool available;
    float min_value;
    float max_value;
    float resolution;
    uint32_t default_rate;
} sensor_def_t;

static sensor_def_t _sensor_defs[SENSOR_TYPE_COUNT] = {
    {SENSOR_TYPE_ACCELEROMETER, "Accelerometer", true, -78.0f, 78.0f, 0.001f, 50},
    {SENSOR_TYPE_GYROSCOPE, "Gyroscope", true, -34.0f, 34.0f, 0.001f, 100},
    {SENSOR_TYPE_MAGNETOMETER, "Magnetometer", true, -800.0f, 800.0f, 0.01f, 10},
    {SENSOR_TYPE_LIGHT, "Light Sensor", true, 0.0f, 40000.0f, 1.0f, 5},
    {SENSOR_TYPE_PROXIMITY, "Proximity Sensor", true, 0.0f, 10.0f, 0.01f, 1},
    {SENSOR_TYPE_PRESSURE, "Barometric Pressure", false, 300.0f, 1100.0f, 0.1f, 1},
    {SENSOR_TYPE_HUMIDITY, "Humidity Sensor", false, 0.0f, 100.0f, 0.1f, 1},
    {SENSOR_TYPE_TEMPERATURE, "Temperature Sensor", false, -40.0f, 125.0f, 0.1f, 1},
    {SENSOR_TYPE_STEP_COUNTER, "Step Counter", true, 0.0f, 99999.0f, 1.0f, 1},
    {SENSOR_TYPE_HEART_RATE, "Heart Rate Sensor", true, 20.0f, 250.0f, 1.0f, 1},
    {SENSOR_TYPE_FINGERPRINT, "Fingerprint Sensor", true, 0.0f, 1.0f, 0.01f, 1},
};

status_t sensor_hal_init(void) {
    printf("[Sensor HAL] Sensör altyapısı başlatılıyor...\n");
    memset(_last_events, 0, sizeof(_last_events));
    _sensor_callback = NULL;

    uint32_t available_count = 0;
    for (uint32_t i = 0; i < SENSOR_TYPE_COUNT; i++) {
        if (_sensor_defs[i].available) {
            available_count++;
            sensor_hal_enable(_sensor_defs[i].type, _sensor_defs[i].default_rate);
        }
    }

    printf("[Sensor HAL] %d sensör hazır (%d aktif)\n",
           SENSOR_TYPE_COUNT, available_count);
    return STATUS_SUCCESS;
}

void sensor_hal_shutdown(void) {
    printf("[Sensor HAL] Sensör altyapısı kapatılıyor...\n");
}

status_t sensor_hal_enable(sensor_type_t type, uint32_t sampling_rate) {
    if (type >= SENSOR_TYPE_COUNT || !_sensor_defs[type].available) {
        return STATUS_INVALID;
    }

    sensor_hal_disable(type);

    memset(&_last_events[type], 0, sizeof(sensor_event_t));
    _last_events[type].type = type;
    _last_events[type].available = true;
    _last_events[type].sampling_rate = sampling_rate;

    printf("[Sensor HAL] %s sensörü etkinleştirildi (%d Hz)\n",
           _sensor_defs[type].name, sampling_rate);
    return STATUS_SUCCESS;
}

status_t sensor_hal_disable(sensor_type_t type) {
    if (type >= SENSOR_TYPE_COUNT) {
        return STATUS_INVALID;
    }
    _last_events[type].available = false;
    return STATUS_SUCCESS;
}

bool sensor_hal_is_available(sensor_type_t type) {
    if (type >= SENSOR_TYPE_COUNT) {
        return false;
    }
    return _sensor_defs[type].available;
}

sensor_event_t* sensor_hal_get_last_event(sensor_type_t type) {
    if (type >= SENSOR_TYPE_COUNT) {
        return NULL;
    }
    return &_last_events[type];
}

status_t sensor_hal_register_callback(sensor_callback_t cb) {
    _sensor_callback = cb;
    printf("[Sensor HAL] Sensör geri bildirimi kaydedildi\n");
    return STATUS_SUCCESS;
}

void sensor_hal_update_events(void) {
    uint64_t now = 0;
    for (uint32_t i = 0; i < SENSOR_TYPE_COUNT; i++) {
        if (!_sensor_defs[i].available || !_last_events[i].available) {
            continue;
        }

        memset(&_last_events[i], 0, sizeof(sensor_event_t));
        _last_events[i].type = _sensor_defs[i].type;
        _last_events[i].available = true;
        _last_events[i].sampling_rate = _sensor_defs[i].default_rate;
        now++;

        if (_sensor_callback) {
            _sensor_callback(_sensor_defs[i].type, &_last_events[i]);
        }
    }
}