/*
 * OmniOS Security Module
 * Permission management, sandbox, encryption helpers
 */

#include "include/omnios_core.h"
#include <string.h>
#include <stdio.h>

#define MAX_PERMISSIONS 32
#define MAX_SANDBOXES 64

typedef enum {
    PERM_CAMERA = 0,
    PERM_MICROPHONE,
    PERM_LOCATION,
    PERM_STORAGE_READ,
    PERM_STORAGE_WRITE,
    PERM_CONTACTS,
    PERM_CALENDAR,
    PERM_NOTIFICATIONS,
    PERM_BLUETOOTH,
    PERM_NETWORK,
    PERM_SENSORS,
    PERM_PHONE,
    PERM_SMS,
    PERM_COUNT
} PermissionType;

static const char* _perm_names[PERM_COUNT] = {
    "camera", "microphone", "location", "storage_read", "storage_write",
    "contacts", "calendar", "notifications", "bluetooth", "network",
    "sensors", "phone", "sms"
};

typedef struct {
    uint32_t pid;
    bool permissions[PERM_COUNT];
    char sandbox_path[256];
    bool is_active;
} Sandbox;

static Sandbox _sandboxes[MAX_SANDBOXES];
static uint32_t _sandbox_count = 0;

/* Permission lookup */
PermissionType om_perm_from_string(const char* name) {
    if (!name) return PERM_COUNT;
    for (int i = 0; i < PERM_COUNT; i++) {
        if (strcmp(_perm_names[i], name) == 0)
            return (PermissionType)i;
    }
    return PERM_COUNT;
}

const char* om_perm_to_string(PermissionType perm) {
    if (perm < PERM_COUNT) return _perm_names[perm];
    return "unknown";
}

uint32_t om_perm_count(void) {
    return PERM_COUNT;
}

/* Sandbox management */
bool om_sandbox_create(uint32_t pid, const char* path) {
    if (_sandbox_count >= MAX_SANDBOXES) return false;

    Sandbox* sb = &_sandboxes[_sandbox_count];
    sb->pid = pid;
    strncpy(sb->sandbox_path, path ? path : "/tmp/omnios/", sizeof(sb->sandbox_path) - 1);

    for (int i = 0; i < PERM_COUNT; i++)
        sb->permissions[i] = false;

    sb->is_active = true;
    _sandbox_count++;
    return true;
}

bool om_sandbox_destroy(uint32_t pid) {
    for (uint32_t i = 0; i < _sandbox_count; i++) {
        if (_sandboxes[i].pid == pid) {
            _sandboxes[i].is_active = false;
            _sandboxes[i] = _sandboxes[_sandbox_count - 1];
            _sandbox_count--;
            return true;
        }
    }
    return false;
}

Sandbox* om_sandbox_get(uint32_t pid) {
    for (uint32_t i = 0; i < _sandbox_count; i++) {
        if (_sandboxes[i].pid == pid && _sandboxes[i].is_active)
            return &_sandboxes[i];
    }
    return NULL;
}

/* Permission checking */
bool om_perm_grant(uint32_t pid, PermissionType perm) {
    Sandbox* sb = om_sandbox_get(pid);
    if (!sb || perm >= PERM_COUNT) return false;
    sb->permissions[perm] = true;
    return true;
}

bool om_perm_revoke(uint32_t pid, PermissionType perm) {
    Sandbox* sb = om_sandbox_get(pid);
    if (!sb || perm >= PERM_COUNT) return false;
    sb->permissions[perm] = false;
    return true;
}

bool om_perm_check(uint32_t pid, PermissionType perm) {
    Sandbox* sb = om_sandbox_get(pid);
    if (!sb || perm >= PERM_COUNT) return false;
    return sb->permissions[perm];
}

uint32_t om_perm_get_granted_count(uint32_t pid) {
    Sandbox* sb = om_sandbox_get(pid);
    if (!sb) return 0;
    uint32_t count = 0;
    for (int i = 0; i < PERM_COUNT; i++) {
        if (sb->permissions[i]) count++;
    }
    return count;
}

/* Security info */
void om_security_get_info(char* buffer, uint32_t buffer_size) {
    if (!buffer || buffer_size < 256) return;

    uint32_t total = 0;
    for (uint32_t i = 0; i < _sandbox_count; i++) {
        if (_sandboxes[i].is_active) total++;
    }

    snprintf(buffer, buffer_size,
        "{\"active_sandboxes\":%u,\"total_permissions\":%d}",
        total, PERM_COUNT);
}
