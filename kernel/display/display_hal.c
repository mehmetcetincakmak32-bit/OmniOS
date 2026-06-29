/*
 * OmniOS Display HAL Implementation
 * Generic display driver interface for mobile devices
 */

#include "display_hal.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Display state */
static bool _display_initialized = false;
static display_capabilities_t _display_caps;
static display_mode_t _display_mode;
static framebuffer_t _framebuffer;
static bool _display_powered = false;
static uint32_t _brightness = 128;
static display_orientation_t _orientation = DISPLAY_ORIENTATION_PORTRAIT;
static uint32_t _max_fps = 60;

static display_vsync_callback_t _vsync_cb = NULL;
static display_hotplug_callback_t _hotplug_cb = NULL;

/* VSync timing */
static uint64_t _last_vsync_time = 0;
static uint32_t _vsync_period_ns = (1000000000ULL / 60);  /* 60Hz */

/* ================================================================
 * Display Initialization
 * ================================================================ */

status_t display_hal_init(void) {
    if (_display_initialized) return STATUS_SUCCESS;

    printf("[Display HAL] Mobil ekran arabirimi baslatiliyor...\n");

    /* Initialize display capabilities */
    memset(&_display_caps, 0, sizeof(display_capabilities_t));
    _display_caps.virtual_width = 2400;
    _display_caps.virtual_height = 1440;
    _display_caps.physical_width = 1080;
    _display_caps.physical_height = 1920;
    _display_caps.x_resolution = 74;
    _display_caps.y_resolution = 131;
    _display_caps.physical_size_mm2 = 74 * 131;
    _display_caps.backing_buffer_count = 3;
    _display_caps.supported_orientations = (
        DISPLAY_ORIENTATION_PORTRAIT | 
        DISPLAY_ORIENTATION_LANDSCAPE
    );
    _display_caps.has_hdr = true;
    _display_caps.has_dynamic_refresh = false;
    _display_caps.x_scale = 2.0f;
    _display_caps.y_scale = 2.0f;
    _display_caps.vsync_period_ns = _vsync_period_ns;

    /* Set default display mode */
    _display_mode.width = _display_caps.physical_width;
    _display_mode.height = _display_caps.physical_height;
    _display_mode.refresh_rate = 60;
    _display_mode.format = DISPLAY_PIXEL_FORMAT_RGB888;
    _display_mode.orientation = _orientation;
    _display_mode.x_resolution = _display_caps.x_resolution;
    _display_mode.y_resolution = _display_caps.y_resolution;
    _display_mode.physical_size_mm2 = _display_caps.physical_size_mm2;
    _display_mode.backing_buffer_count = _display_caps.backing_buffer_count;

    /* Initialize framebuffer (simulated) */
    _framebuffer.width = _display_mode.width;
    _framebuffer.height = _display_mode.height;
    _framebuffer.bpp = (32 * _display_mode.format + 7) / 8;
    _framebuffer.stride = _framebuffer.width * _framebuffer.bpp;
    _framebuffer.size = _framebuffer.stride * _framebuffer.height;
    _framebuffer.virtual_address = NULL;
    _framebuffer.physical_address = 0x10000000;
    _framebuffer.handle = 0x10001;

    printf("[Display HAL] Ekran: %dx%d %s %dHz\n",
           _display_mode.width, _display_mode.height,
           _display_mode.format == DISPLAY_PIXEL_FORMAT_RGB888 ? "RGB888" : "unknown",
           _display_mode.refresh_rate);

    _display_initialized = true;
    _display_powered = false;

    return STATUS_SUCCESS;
}

status_t display_hal_shutdown(void) {
    if (!_display_initialized) return STATUS_SUCCESS;

    printf("[Display HAL] Ekran kapatiliyor...\n");

    /* Cleanup framebuffer */
    memset(&_framebuffer, 0, sizeof(framebuffer_t));
    _display_powered = false;
    _display_initialized = false;

    return STATUS_SUCCESS;
}

/* ================================================================
 * Display Configuration
 * ================================================================ */

status_t display_hal_get_capabilities(display_capabilities_t* caps) {
    if (!_display_initialized || !caps) return STATUS_INVALID;
    memcpy(caps, &_display_caps, sizeof(display_capabilities_t));
    return STATUS_SUCCESS;
}

status_t display_hal_get_current_mode(display_mode_t* mode) {
    if (!_display_initialized || !mode) return STATUS_INVALID;
    memcpy(mode, &_display_mode, sizeof(display_mode_t));
    return STATUS_SUCCESS;
}

status_t display_hal_set_mode(uint32_t width, uint32_t height,
                              display_pixel_format_t format, uint32_t refresh_rate) {
    if (!_display_initialized) return STATUS_INVALID;

    /* Validate mode */
    if (format > DISPLAY_PIXEL_FORMAT_BGRA8888) return STATUS_INVALID;
    if (refresh_rate < 30 || refresh_rate > 120) return STATUS_INVALID;
    if (width > _display_caps.virtual_width || height > _display_caps.virtual_height)
        return STATUS_INVALID;

    /* Update display mode */
    _display_mode.width = width;
    _display_mode.height = height;
    _display_mode.format = format;
    _display_mode.refresh_rate = refresh_rate;
    _display_mode.orientation = _orientation;
    _display_mode.backing_buffer_count = _display_caps.backing_buffer_count;

    /* Update framebuffer */
    _framebuffer.width = width;
    _framebuffer.height = height;
    _framebuffer.bpp = (32 * format + 7) / 8;
    _framebuffer.stride = _framebuffer.width * _framebuffer.bpp;
    _framebuffer.size = _framebuffer.stride * _framebuffer.height;

    printf("[Display HAL] Ekran modu degistirildi: %dx%d %s %dHz\n",
           width, height, format == DISPLAY_PIXEL_FORMAT_RGB888 ? "RGB888" : "unknown", refresh_rate);

    /* Update vsync period */
    _vsync_period_ns = (1000000000ULL / refresh_rate);
    _display_caps.vsync_period_ns = _vsync_period_ns;

    return STATUS_SUCCESS;
}

status_t display_hal_get_framebuffer(framebuffer_t* fb) {
    if (!_display_initialized || !fb) return STATUS_INVALID;
    memcpy(fb, &_framebuffer, sizeof(framebuffer_t));
    return STATUS_SUCCESS;
}

/* ================================================================
 * Display Operations
 * ================================================================ */

status_t display_hal_blit(const void* pixels, uint32_t x, uint32_t y,
                          uint32_t width, uint32_t height) {
    if (!_display_initialized || !pixels) return STATUS_INVALID;

    /* Validate coordinates */
    if (x >= _framebuffer.width || y >= _framebuffer.height) return STATUS_INVALID;
    if (x + width > _framebuffer.width || y + height > _framebuffer.height) return STATUS_INVALID;

    /* Simulate blitting operation */
    uint32_t pixels_count = width * height;
    uint32_t bytes_per_pixel = _framebuffer.bpp;
    uint32_t bytes_to_copy = pixels_count * bytes_per_pixel;

    printf("[Display HAL] Blit %dx%d at (%d,%d) %d bytes\n",
           width, height, x, y, bytes_to_copy);

    /* In real implementation: copy pixels to framebuffer */
    return STATUS_SUCCESS;
}

status_t display_hal_clear(uint32_t color) {
    printf("[Display HAL] Ekran siliniyor renk=0x%08X\n", color);
    return STATUS_SUCCESS;
}

/* ================================================================
 * Power Management
 * ================================================================ */

status_t display_hal_set_power(bool on) {
    _display_powered = on;
    printf("[Display HAL] Ekran %s\n", on ? "acildi" : "kapatildi");
    return STATUS_SUCCESS;
}

bool display_hal_is_powered(void) { return _display_powered; }

status_t display_hal_set_brightness(uint32_t level) {
    if (level > 255) level = 255;
    _brightness = level;
    printf("[Display HAL] Parlaklik: %d (%d%%)\n", level, (level * 100) / 255);
    return STATUS_SUCCESS;
}

uint32_t display_hal_get_brightness(void) { return _brightness; }

status_t display_hal_set_standby(uint32_t timeout_ms) {
    printf("[Display HAL] Ekran hazirlama modu: %dms\n", timeout_ms);
    return STATUS_SUCCESS;
}

/* ================================================================
 * Display Configuration
 * ================================================================ */

status_t display_hal_set_orientation(display_orientation_t orientation) {
    if (!_display_initialized) return STATUS_INVALID;
    if (!(orientation & _display_caps.supported_orientations)) return STATUS_INVALID;

    _orientation = orientation;
    _display_mode.orientation = orientation;

    printf("[Display HAL] Yon degistirildi: %d\n", orientation);
    return STATUS_SUCCESS;
}

display_orientation_t display_hal_get_orientation(void) { return _orientation; }

status_t display_hal_set_fps_limit(uint32_t max_fps) {
    if (max_fps < 1 || max_fps > 120) return STATUS_INVALID;
    _max_fps = max_fps;

    uint32_t current_rate = _display_mode.refresh_rate;
    if (current_rate > _max_fps) {
        display_hal_set_mode(_display_mode.width, _display_mode.height,
                            _display_mode.format, _max_fps);
    }

    printf("[Display HAL] FPS siniri: %d\n", max_fps);
    return STATUS_SUCCESS;
}

uint32_t display_hal_get_fps_limit(void) { return _max_fps; }

/* ================================================================
 * VSync and Events
 * ================================================================ */

uint64_t timer_get_uptime(void);

status_t display_hal_register_vsync_callback(display_vsync_callback_t cb) {
    _vsync_cb = cb;
    printf("[Display HAL] VSync callback kaydedildi\n");
    return STATUS_SUCCESS;
}

status_t display_hal_register_hotplug_callback(display_hotplug_callback_t cb) {
    _hotplug_cb = cb;
    printf("[Display HAL] Hotplug callback kaydedildi\n");
    return STATUS_SUCCESS;
}

uint32_t display_hal_get_vsync_period_ns(void) { return _vsync_period_ns; }

/* ================================================================
 * HDR Support
 * ================================================================ */

status_t display_hal_set_hdr(bool enable) {
    printf("[Display HAL] HDR %s\n", enable ? "acildi" : "kapatildi");
    return STATUS_SUCCESS;
}

bool display_hal_is_hdr_enabled(void) { return _display_caps.has_hdr; }

status_t display_hal_set_hdr_mastering_display_color_volume(const char* metadata) {
    (void)metadata;
    return STATUS_SUCCESS;
}

status_t display_hal_get_current_refresh_rate(uint32_t* refresh_rate) {
    if (!refresh_rate) return STATUS_INVALID;
    *refresh_rate = _display_mode.refresh_rate;
    return STATUS_SUCCESS;
}

/* ================================================================
 * Debug Information
 * ================================================================ */

status_t display_hal_get_debug_info(char* buffer, uint32_t buffer_size) {
    if (!buffer || buffer_size < 1024) return STATUS_INVALID;

    snprintf(buffer, buffer_size,
             "Display HAL Debug Info:\n"
             "  Initialized: %s\n"
             "  Powered: %s\n"
             "  Mode: %dx%d %s %dHz\n"
             "  Brightness: %d (%.1f%%)\n"
             "  Orientation: %d\n"
             "  FPS Limit: %d\n"
             "  VSync Period: %d ns\n"
             "  Backbuffer Count: %d\n"
             "  Backbuffer Size: %d x %d x %d bytes\n",
             _display_initialized ? "yes" : "no",
             _display_powered ? "yes" : "no",
             _display_mode.width, _display_mode.height,
             _display_mode.format == DISPLAY_PIXEL_FORMAT_RGB888 ? "RGB888" : "unknown",
             _display_mode.refresh_rate,
             _brightness, (double)_brightness * 100.0 / 255.0,
             _orientation,
             _max_fps,
             _vsync_period_ns,
             _display_mode.backing_buffer_count,
             _framebuffer.width, _framebuffer.height, _framebuffer.size);

    printf("[Display HAL] Debug info print edildi\n");
    return STATUS_SUCCESS;
}

/* ================================================================
 * Internal Callbacks
 * ================================================================ */

void display_hal_notify_vsync(void) {
    uint64_t now = timer_get_uptime();
    uint64_t elapsed = now - _last_vsync_time;

    if (_vsync_cb && elapsed >= _vsync_period_ns) {
        _last_vsync_time = now;
        _vsync_cb(now);
    }
}

void display_hal_notify_hotplug(bool connected) {
    if (_hotplug_cb) {
        _hotplug_cb(connected);
    }
}