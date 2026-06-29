/*
 * OmniOS Runtime Loader
 * Platform-agnostic application runtime loader
 */

#include "include/omnios_core.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    PlatformType type;
    const char*  name;
    const char*  version;
    bool         available;
    uint32_t     api_level;
} RuntimeModule;

static RuntimeModule _runtimes[] = {
    { PLATFORM_ANDROID, "Android Runtime (ART)",  "4.0",  true,  35 },
    { PLATFORM_IOS,     "iOS Runtime (UIKit)",    "3.2",  true,  18 },
    { PLATFORM_CROSS,   "OmniOS Native Runtime",  "1.0",  true,  1  },
    { PLATFORM_UNKNOWN, NULL,                     NULL,   false, 0  },
};

PlatformType om_runtime_detect(const char* app_path) {
    if (!app_path) return PLATFORM_UNKNOWN;

    size_t len = strlen(app_path);

    /* APK files -> Android */
    if (len >= 4 &&
        (app_path[len-4] == '.' || app_path[len-4] == '\0') &&
        (app_path[len-3] == 'a' || app_path[len-3] == 'A') &&
        (app_path[len-2] == 'p' || app_path[len-2] == 'P') &&
        (app_path[len-1] == 'k' || app_path[len-1] == 'K'))
        return PLATFORM_ANDROID;

    /* IPA files -> iOS */
    if (len >= 4 &&
        (app_path[len-4] == '.' || app_path[len-4] == '\0') &&
        (app_path[len-3] == 'i' || app_path[len-3] == 'I') &&
        (app_path[len-2] == 'p' || app_path[len-2] == 'P') &&
        (app_path[len-1] == 'a' || app_path[len-1] == 'A'))
        return PLATFORM_IOS;

    /* OMNIPKG files -> Cross */
    if (len >= 9 &&
        strncmp(app_path + len - 9, ".omnipkg", 8) == 0)
        return PLATFORM_CROSS;

    return PLATFORM_CROSS;
}

const char* om_runtime_get_name(PlatformType platform) {
    for (int i = 0; _runtimes[i].name != NULL; i++) {
        if (_runtimes[i].type == platform)
            return _runtimes[i].name;
    }
    return "Unknown Runtime";
}

const char* om_runtime_get_version(PlatformType platform) {
    for (int i = 0; _runtimes[i].name != NULL; i++) {
        if (_runtimes[i].type == platform)
            return _runtimes[i].version;
    }
    return "0.0";
}

uint32_t om_runtime_get_api_level(PlatformType platform) {
    for (int i = 0; _runtimes[i].name != NULL; i++) {
        if (_runtimes[i].type == platform)
            return _runtimes[i].api_level;
    }
    return 0;
}

bool om_runtime_is_available(PlatformType platform) {
    for (int i = 0; _runtimes[i].name != NULL; i++) {
        if (_runtimes[i].type == platform)
            return _runtimes[i].available;
    }
    return false;
}

const char* om_runtime_platform_name(PlatformType platform) {
    switch (platform) {
        case PLATFORM_ANDROID: return "Android";
        case PLATFORM_IOS:     return "iOS";
        case PLATFORM_CROSS:   return "OmniOS";
        default:               return "Unknown";
    }
}

bool om_runtime_can_run(PlatformType runtime, PlatformType app_platform) {
    if (runtime == app_platform) return true;
    if (app_platform == PLATFORM_CROSS) return true;
    return false;
}
