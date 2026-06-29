/*
 * OmniOS Display HAL
 * Display driver interface for mobile device
 */

#ifndef OMNOS_DISPLAY_HAL_H
#define OMNOS_DISPLAY_HAL_H

#include "../include/omnios_kernel.h"

/* ================================================================
 * Display Controller
 * ================================================================ */

typedef enum {
    DISPLAY_ORIENTATION_PORTRAIT   = 0,
    DISPLAY_ORIENTATION_LANDSCAPE  = 1,
    DISPLAY_ORIENTATION_REVERSE_PORTRAIT = 2,
    DISPLAY_ORIENTATION_REVERSE_LANDSCAPE = 3,
} display_orientation_t;

typedef enum {
    DISPLAY_PIXEL_FORMAT_RGB888   = 0,
    DISPLAY_PIXEL_FORMAT_RGB565   = 1,
    DISPLAY_PIXEL_FORMAT_RGB666   = 2,
    DISPLAY_PIXEL_FORMAT_BGR888   = 3,
    DISPLAY_PIXEL_FORMAT_ARGB8888 = 4,
    DISPLAY_PIXEL_FORMAT_ABGR8888 = 5,
    DISPLAY_PIXEL_FORMAT_RGBA8888 = 6,
    DISPLAY_PIXEL_FORMAT_BGRA8888 = 7,
} display_pixel_format_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t refresh_rate;   /* Hz */
    display_pixel_format_t format;
    display_orientation_t orientation;
    uint32_t x_resolution;   /* mm */
    uint32_t y_resolution;   /* mm */
    float    physical_size_mm2;
    uint32_t backing_buffer_count;
} display_mode_t;

typedef struct {
    uint32_t virtual_width;   /* Virtual/pan size */
    uint32_t virtual_height;
    uint32_t physical_width;  /* Actual display size */
    uint32_t physical_height;
    int32_t  x_offset;
    int32_t  y_offset;
    double   x_scale;        /* Display density scale */
    double   y_scale;
    uint32_t vsync_period_ns;
    display_orientation_t supported_orientations;
    bool     has_hdr;
    bool     has_dynamic_refresh;  /* Variable refresh rate support */
} display_capabilities_t;

/* Framebuffer management */
typedef struct {
    uintptr_t handle;        /* Internal driver handle */
    uint32_t  width;
    uint32_t  height;
    uint32_t  bpp;
    uint32_t  stride;
    void*     virtual_address;
    uintptr_t physical_address;
    uint32_t  size;
} framebuffer_t;

typedef void (*display_vsync_callback_t)(uint64_t timestamp);
typedef void (*display_hotplug_callback_t)(bool connected);

/* Display controller interfaces */
status_t display_hal_init(void);
status_t display_hal_shutdown(void);
status_t display_hal_get_capabilities(display_capabilities_t* caps);
status_t display_hal_get_current_mode(display_mode_t* mode);
status_t display_hal_set_mode(uint32_t width, uint32_t height, 
                              display_pixel_format_t format, uint32_t refresh_rate);
status_t display_hal_get_framebuffer(framebuffer_t* fb);
status_t display_hal_blit(const void* pixels, uint32_t x, uint32_t y,
                          uint32_t width, uint32_t height);
status_t display_hal_clear(uint32_t color);

/* Power management */
status_t display_hal_set_power(bool on);
bool display_hal_is_powered(void);
status_t display_hal_set_brightness(uint32_t level);  /* 0-255 */
uint32_t display_hal_get_brightness(void);
status_t display_hal_set_standby(uint32_t timeout_ms);

/* Display configuration */
status_t display_hal_set_orientation(display_orientation_t orientation);
display_orientation_t display_hal_get_orientation(void);
status_t display_hal_set_fps_limit(uint32_t max_fps);
uint32_t display_hal_get_fps_limit(void);

/* VSync and events */
status_t display_hal_register_vsync_callback(display_vsync_callback_t cb);
status_t display_hal_register_hotplug_callback(display_hotplug_callback_t cb);
uint32_t display_hal_get_vsync_period_ns(void);

/* HDR support */
status_t display_hal_set_hdr(bool enable);
bool display_hal_is_hdr_enabled(void);
status_t display_hal_set_hdr_b Mastering Display Color Volume (DMCC) metadata (TODO)
status_t display_hal_get_current_refresh_rate(uint32_t* refresh_rate);

/* Debug information */
status_t display_hal_get_debug_info(char* buffer, uint32_t buffer_size);

#endif /* OMNOS_DISPLAY_HAL_H */
