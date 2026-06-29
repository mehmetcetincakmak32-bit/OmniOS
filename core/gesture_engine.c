/*
 * OmniOS Gesture Engine
 * High-performance gesture recognition and mapping
 */

#include "include/omnios_core.h"
#include <string.h>

#define MAX_GESTURES 16
#define MAX_HISTORY 64

static const struct {
    GestureType type;
    const char* name;
    const char* action_normal;
    const char* action_flow;
    const char* description;
} _gesture_defs[] = {
    { GESTURE_SWIPE_UP,    "swipe_up",    "open_app_menu",       "open_app_menu",       "Uygulama menusu" },
    { GESTURE_SWIPE_RIGHT, "swipe_right", "open_recent_apps",    "open_recent_apps",    "Son uygulamalar" },
    { GESTURE_SWIPE_LEFT,  "swipe_left",  "open_notifications",  "open_notifications",  "Bildirimler" },
    { GESTURE_SWIPE_DOWN,  "swipe_down",  "open_quick_settings", "open_quick_settings",  "Hizli ayarlar" },
    { GESTURE_DOUBLE_TAP,  "double_tap",  "go_home",             "go_home",             "Ana ekran" },
    { GESTURE_LONG_PRESS,  "long_press",  "voice_command",       "voice_command",       "Sesli komut" },
    { GESTURE_PINCH_OUT,   "pinch_out",   "app_selector",        "app_selector_flow",   "Uygulama secici" },
    { GESTURE_PINCH_IN,    "pinch_in",    "zoom_out",            "minimize",            "Kucult" },
    { GESTURE_TAP,         "tap",         "select",              "select_flow",         "Sec" },
    { GESTURE_NONE,        NULL,          NULL,                  NULL,                  NULL }
};

static GestureMapping _custom_mappings[MAX_GESTURES];
static uint32_t _custom_count = 0;

static GestureType _history[MAX_HISTORY];
static uint32_t _history_idx = 0;

GestureType om_gesture_recognize(const char* name) {
    if (!name) return GESTURE_NONE;

    for (int i = 0; _gesture_defs[i].name != NULL; i++) {
        if (strcmp(_gesture_defs[i].name, name) == 0)
            return _gesture_defs[i].type;
    }
    return GESTURE_NONE;
}

const char* om_gesture_get_action(GestureType gesture, UIMode mode) {
    for (int i = 0; _gesture_defs[i].name != NULL; i++) {
        if (_gesture_defs[i].type == gesture) {
            return (mode == MODE_FLOW)
                ? _gesture_defs[i].action_flow
                : _gesture_defs[i].action_normal;
        }
    }

    for (uint32_t i = 0; i < _custom_count; i++) {
        if (_custom_mappings[i].gesture == gesture &&
            _custom_mappings[i].mode == mode) {
            return _custom_mappings[i].action;
        }
    }

    return "no_action";
}

bool om_gesture_register_mapping(GestureMapping mapping) {
    if (_custom_count >= MAX_GESTURES) return false;

    _custom_mappings[_custom_count++] = mapping;
    return true;
}

uint32_t om_gesture_get_supported(GestureType* buffer, uint32_t max) {
    uint32_t count = 0;
    for (int i = 0; _gesture_defs[i].name != NULL && count < max; i++) {
        buffer[count++] = _gesture_defs[i].type;
    }
    return count;
}

void om_gesture_log(GestureType gesture) {
    _history[_history_idx % MAX_HISTORY] = gesture;
    _history_idx++;
}

const char* om_gesture_get_name(GestureType gesture) {
    for (int i = 0; _gesture_defs[i].name != NULL; i++) {
        if (_gesture_defs[i].type == gesture)
            return _gesture_defs[i].name;
    }
    return "unknown";
}
