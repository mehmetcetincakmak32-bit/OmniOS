/*
 * OmniOS Mobile Hardware Abstraction Layer
 * Unified interface for phone hardware components
 */

#ifndef OMNOS_MOBILE_HAL_H
#define OMNOS_MOBILE_HAL_H

#include "../include/omnios_kernel.h"

/* ================================================================
 * Display
 * ================================================================ */

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;          /* Bits per pixel */
    uint32_t pitch;        /* Bytes per line */
    uintptr_t framebuffer; /* Physical address */
    uint32_t refresh_rate; /* Hz */
    uint32_t orientation;  /* 0=portrait, 1=landscape */
    bool double_buffered;
} display_info_t;

typedef enum {
    DISPLAY_PANEL_LCD     = 0,
    DISPLAY_PANEL_OLED    = 1,
    DISPLAY_PANEL_AMOLED  = 2,
} display_panel_type_t;

status_t hal_display_init(void);
status_t hal_display_get_info(display_info_t* info);
status_t hal_display_set_brightness(uint32_t level);  /* 0-255 */
status_t hal_display_set_power(bool on);
status_t hal_display_swap_buffers(void);
uint32_t hal_display_get_brightness(void);
display_panel_type_t hal_display_get_panel_type(void);

/* ================================================================
 * Touchscreen
 * ================================================================ */

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t pressure;    /* 0-255 */
    uint8_t  finger_id;
    bool     touch;       /* true=down, false=up */
} touch_event_t;

typedef void (*touch_callback_t)(const touch_event_t* event, uint32_t count);

status_t hal_touch_init(void);
status_t hal_touch_register_callback(touch_callback_t cb);
status_t hal_touch_set_power(bool on);
uint32_t hal_touch_get_max_fingers(void);
bool     hal_touch_is_active(void);

/* ================================================================
 * Battery / Power
 * ================================================================ */

typedef struct {
    uint32_t level;        /* 0-100 percentage */
    int32_t  voltage_mv;   /* mV */
    int32_t  current_ma;   /* mA (+=charging, -=discharging) */
    int32_t  temperature_c; /* Celsius */
    bool     charging;
    bool     usb_connected;
    bool     wireless_charging;
    uint32_t capacity_mah;
    uint32_t remaining_capacity_mah;
    uint32_t time_to_empty_min;
    uint32_t time_to_full_min;
    uint8_t  health;       /* 0=unknown, 1=good, 2=degraded, 3=bad */
} battery_info_t;

status_t hal_battery_init(void);
status_t hal_battery_get_info(battery_info_t* info);
status_t hal_battery_set_charging(bool enable);
uint32_t hal_battery_get_level(void);
bool     hal_battery_is_charging(void);

/* ================================================================
 * Sensors
 * ================================================================ */

typedef enum {
    SENSOR_ACCELEROMETER = 0,
    SENSOR_GYROSCOPE     = 1,
    SENSOR_MAGNETOMETER  = 2,
    SENSOR_LIGHT         = 3,
    SENSOR_PROXIMITY     = 4,
    SENSOR_PRESSURE      = 5,
    SENSOR_HUMIDITY      = 6,
    SENSOR_TEMPERATURE   = 7,
    SENSOR_STEP_COUNTER  = 8,
    SENSOR_HEART_RATE    = 9,
    SENSOR_FINGERPRINT   = 10,
    SENSOR_IR_GESTURE    = 11,
    SENSOR_COUNT
} sensor_type_t;

typedef struct {
    sensor_type_t type;
    float         x, y, z;   /* 3-axis data */
    float         value;     /* Single value (light, proximity, etc.) */
    uint64_t      timestamp;
    uint32_t      accuracy;  /* 0=unreliable, 1=low, 2=medium, 3=high */
} sensor_event_t;

typedef void (*sensor_callback_t)(sensor_type_t type, const sensor_event_t* event);

status_t hal_sensor_init(void);
status_t hal_sensor_enable(sensor_type_t type, uint32_t sampling_rate_hz);
status_t hal_sensor_disable(sensor_type_t type);
status_t hal_sensor_register_callback(sensor_callback_t cb);
bool     hal_sensor_is_available(sensor_type_t type);
uint32_t hal_sensor_get_max_range(sensor_type_t type);
float    hal_sensor_get_resolution(sensor_type_t type);

/* ================================================================
 * Camera
 * ================================================================ */

#define CAMERA_MAX_SENSORS  4

typedef struct {
    uint32_t  sensor_id;
    char      name[32];
    uint32_t  max_width;
    uint32_t  max_height;
    float     focal_length_mm;
    float     aperture;
    bool      flash;
    bool      autofocus;
    bool      optical_stabilization;
} camera_sensor_info_t;

typedef struct {
    uint32_t   width;
    uint32_t   height;
    uint32_t   format;       /* 0=RGB, 1=YUV, 2=RAW, 3=JPEG */
    uint8_t*   buffer;
    uint32_t   buffer_size;
    uint32_t   stride;
} camera_frame_t;

typedef void (*camera_callback_t)(uint32_t sensor_id, const camera_frame_t* frame);

status_t hal_camera_init(void);
status_t hal_camera_get_sensor_info(uint32_t sensor_id, camera_sensor_info_t* info);
uint32_t hal_camera_get_sensor_count(void);
status_t hal_camera_start_preview(uint32_t sensor_id, uint32_t width, uint32_t height);
status_t hal_camera_stop_preview(uint32_t sensor_id);
status_t hal_camera_capture_image(uint32_t sensor_id, camera_frame_t* frame);
status_t hal_camera_register_callback(camera_callback_t cb);

/* ================================================================
 * Vibrator / Haptic
 * ================================================================ */

status_t hal_vibrator_init(void);
status_t hal_vibrator_on(uint32_t duration_ms);
status_t hal_vibrator_off(void);
status_t hal_vibrator_set_amplitude(uint8_t level); /* 0-255 */
status_t hal_vibrator_pattern(const uint32_t* pattern, uint32_t count);

/* ================================================================
 * LED / Notification Light
 * ================================================================ */

status_t hal_led_init(void);
status_t hal_led_set_color(uint8_t r, uint8_t g, uint8_t b);
status_t hal_led_set_brightness(uint8_t level);
status_t hal_led_blink(uint32_t on_ms, uint32_t off_ms, uint32_t count);

/* ================================================================
 * NFC
 * ================================================================ */

typedef void (*nfc_callback_t)(const uint8_t* data, uint32_t len);

status_t hal_nfc_init(void);
status_t hal_nfc_read(uint8_t* data, uint32_t* len);
status_t hal_nfc_write(const uint8_t* data, uint32_t len);
status_t hal_nfc_register_callback(nfc_callback_t cb);
bool     hal_nfc_is_present(void);

/* ================================================================
 * Wi-Fi (WLAN)
 * ================================================================ */

typedef struct {
    char    ssid[32];
    uint8_t bssid[6];
    int8_t  rssi;         /* dBm */
    uint8_t channel;
    uint8_t encryption;   /* 0=none, 1=WEP, 2=WPA, 3=WPA2, 4=WPA3 */
} wifi_scan_result_t;

typedef void (*wifi_scan_callback_t)(const wifi_scan_result_t* results, uint32_t count);

status_t hal_wifi_init(void);
status_t hal_wifi_scan(wifi_scan_callback_t cb);
status_t hal_wifi_connect(const char* ssid, const char* password);
status_t hal_wifi_disconnect(void);
bool     hal_wifi_is_connected(void);
int8_t   hal_wifi_get_rssi(void);

/* ================================================================
 * Bluetooth
 * ================================================================ */

status_t hal_bluetooth_init(void);
status_t hal_bluetooth_enable(void);
status_t hal_bluetooth_disable(void);
bool     hal_bluetooth_is_enabled(void);

/* ================================================================
 * GPS / Location
 * ================================================================ */

typedef struct {
    double  latitude;
    double  longitude;
    double  altitude;
    float   accuracy;
    float   speed;
    float   bearing;
    uint8_t satellites;  /* Number of visible satellites */
    uint64_t timestamp;
} gps_data_t;

typedef void (*gps_callback_t)(const gps_data_t* data);

status_t hal_gps_init(void);
status_t hal_gps_start(uint32_t interval_ms);
status_t hal_gps_stop(void);
status_t hal_gps_register_callback(gps_callback_t cb);
bool     hal_gps_is_fixed(void);

/* ================================================================
 * GPIO / Buttons
 * ================================================================ */

typedef enum {
    BUTTON_POWER     = 0,
    BUTTON_VOLUME_UP = 1,
    BUTTON_VOLUME_DN = 2,
    BUTTON_HOME      = 3,
    BUTTON_BACK      = 4,
    BUTTON_MENU      = 5,
} button_t;

typedef void (*button_callback_t)(button_t button, bool pressed);

status_t hal_buttons_init(void);
status_t hal_buttons_register_callback(button_callback_t cb);

/* ================================================================
 * Initialization
 * ================================================================ */

typedef struct {
    bool enable_display;
    bool enable_touch;
    bool enable_sensors;
    bool enable_camera;
    bool enable_wifi;
    bool enable_bluetooth;
    bool enable_gps;
    bool enable_nfc;
    uint32_t display_width;
    uint32_t display_height;
} hal_config_t;

#define HAL_DEFAULT_CONFIG { \
    .enable_display = true, \
    .enable_touch = true, \
    .enable_sensors = true, \
    .enable_camera = false, \
    .enable_wifi = true, \
    .enable_bluetooth = true, \
    .enable_gps = true, \
    .enable_nfc = false, \
    .display_width = 1080, \
    .display_height = 2340, \
}

status_t hal_init(const hal_config_t* config);
void     hal_shutdown(void);

#endif /* OMNOS_MOBILE_HAL_H */
