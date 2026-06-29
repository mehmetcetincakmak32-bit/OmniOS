/*
 * OmniOS Core
 * Performance-critical components for the OmniOS compatibility layer
 */

#ifndef OMNOS_CORE_H
#define OMNOS_CORE_H

#include <stdint.h>
#include <stdbool.h>

/* Platform identifiers */
typedef enum {
    PLATFORM_UNKNOWN = 0,
    PLATFORM_ANDROID = 1,
    PLATFORM_IOS     = 2,
    PLATFORM_CROSS   = 3
} PlatformType;

/* System states */
typedef enum {
    STATE_BOOTING   = 0,
    STATE_IDLE      = 1,
    STATE_RUNNING   = 2,
    STATE_SLEEPING  = 3,
    STATE_SHUTDOWN  = 4
} SystemState;

/* UI Modes */
typedef enum {
    MODE_NORMAL = 0,
    MODE_FLOW   = 1
} UIMode;

/* Gesture types */
typedef enum {
    GESTURE_NONE        = 0,
    GESTURE_SWIPE_UP    = 1,
    GESTURE_SWIPE_RIGHT = 2,
    GESTURE_SWIPE_LEFT  = 3,
    GESTURE_SWIPE_DOWN  = 4,
    GESTURE_DOUBLE_TAP  = 5,
    GESTURE_LONG_PRESS  = 6,
    GESTURE_PINCH_IN    = 7,
    GESTURE_PINCH_OUT   = 8,
    GESTURE_TAP         = 9
} GestureType;

/* Process states */
typedef enum {
    PROC_CREATED    = 0,
    PROC_RUNNING    = 1,
    PROC_BACKGROUND = 2,
    PROC_SUSPENDED  = 3,
    PROC_TERMINATED = 4,
    PROC_CRASHED    = 5
} ProcessState;

/* Process control block */
typedef struct {
    uint32_t    pid;
    char        name[64];
    PlatformType platform;
    ProcessState state;
    float       cpu_usage;
    float       mem_usage;
    uint64_t    created_at;
    uint64_t    running_time;
    uint8_t     priority;
} Process;

/* Gesture action mapping */
typedef struct {
    GestureType gesture;
    char        action[32];
    char        description[64];
    UIMode      mode;
} GestureMapping;

/* System configuration */
typedef struct {
    uint32_t    max_processes;
    uint32_t    total_memory_mb;
    uint32_t    screen_width;
    uint32_t    screen_height;
    bool        dark_mode;
    bool        gestures_enabled;
    char        device_name[128];
} SystemConfig;

/* Core API */

/* State management */
void        om_system_init(SystemConfig* config);
void        om_system_shutdown(void);
SystemState om_system_get_state(void);
bool        om_system_set_state(SystemState state);

/* Mode management */
UIMode      om_mode_get_current(void);
bool        om_mode_set(UIMode mode);
UIMode      om_mode_toggle(void);
const char* om_mode_get_name(UIMode mode);

/* Process management */
Process*    om_process_create(const char* name, PlatformType platform);
bool        om_process_kill(uint32_t pid);
bool        om_process_suspend(uint32_t pid);
bool        om_process_resume(uint32_t pid);
Process*    om_process_get(uint32_t pid);
uint32_t    om_process_count(void);
void        om_process_list(Process* buffer, uint32_t* count);

/* Gesture processing */
GestureType om_gesture_recognize(const char* name);
const char* om_gesture_get_action(GestureType gesture, UIMode mode);
bool        om_gesture_register_mapping(GestureMapping mapping);
uint32_t    om_gesture_get_supported(GestureType* buffer, uint32_t max);

/* Memory management */
bool        om_memory_allocate(uint32_t pid, uint32_t size_mb);
bool        om_memory_free(uint32_t pid);
uint32_t    om_memory_get_used(void);
uint32_t    om_memory_get_available(void);
float       om_memory_get_usage_percent(void);

/* System info */
void        om_system_get_info(char* buffer, uint32_t buffer_size);
uint32_t    om_system_get_uptime(void);

/* API Translation */
const char* om_api_ios_to_android(const char* ios_class);
const char* om_api_android_to_ios(const char* android_class);
uint32_t    om_api_get_translation_count(void);
int         om_api_translate_method(const char* platform, const char* class_name,
                                    const char* method, char* out, uint32_t out_size);
const char* om_api_get_runtime(PlatformType platform);
int         om_api_get_api_level(PlatformType platform);
bool        om_api_is_compatible(PlatformType app, PlatformType runtime);

/* Security */
typedef int PermissionType;
#define PERM_CAMERA         0
#define PERM_MICROPHONE     1
#define PERM_LOCATION       2
#define PERM_STORAGE_READ   3
#define PERM_STORAGE_WRITE  4
#define PERM_CONTACTS       5
#define PERM_CALENDAR       6
#define PERM_NOTIFICATIONS  7
#define PERM_BLUETOOTH      8
#define PERM_NETWORK        9
#define PERM_SENSORS        10
#define PERM_PHONE          11
#define PERM_SMS            12

PermissionType om_perm_from_string(const char* name);
const char*    om_perm_to_string(PermissionType perm);
uint32_t       om_perm_count(void);
bool           om_sandbox_create(uint32_t pid, const char* path);
bool           om_sandbox_destroy(uint32_t pid);
bool           om_perm_grant(uint32_t pid, PermissionType perm);
bool           om_perm_revoke(uint32_t pid, PermissionType perm);
bool           om_perm_check(uint32_t pid, PermissionType perm);
uint32_t       om_perm_get_granted_count(uint32_t pid);
void           om_security_get_info(char* buffer, uint32_t buffer_size);

#endif /* OMNOS_CORE_H */
