/* OmniOS — userspace/security/capability_protocol.h */
/* Faz 2 - Capability Token IPC */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_CAP_PROTOCOL_H
#define OMNIOS_CAP_PROTOCOL_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/un.h>

#define OMNIOS_CAP_SOCK_PATH  "/run/omnios/cap.sock"
#define OMNIOS_CAP_MAX_MSG   4096
#define OMNIOS_CAP_TOKEN_MAX 64
#define OMNIOS_CAP_LABEL_LEN 64

/* ── Token rights (bitmask) ──────────────────────────────────────── */

typedef uint64_t omnios_cap_rights_t;

#define OMNIOS_CAP_MEMORY_READ     (1ULL << 0)
#define OMNIOS_CAP_MEMORY_WRITE    (1ULL << 1)
#define OMNIOS_CAP_MEMORY_EXEC     (1ULL << 2)
#define OMNIOS_CAP_IPC_SEND        (1ULL << 3)
#define OMNIOS_CAP_IPC_RECV        (1ULL << 4)
#define OMNIOS_CAP_FILE_READ       (1ULL << 5)
#define OMNIOS_CAP_FILE_WRITE      (1ULL << 6)
#define OMNIOS_CAP_NET_CONNECT     (1ULL << 7)
#define OMNIOS_CAP_NET_BIND        (1ULL << 8)
#define OMNIOS_CAP_DISPLAY_ACCESS  (1ULL << 9)
#define OMNIOS_CAP_INPUT_ACCESS    (1ULL << 10)
#define OMNIOS_CAP_AUDIO_PLAYBACK  (1ULL << 11)
#define OMNIOS_CAP_AUDIO_CAPTURE   (1ULL << 12)
#define OMNIOS_CAP_SENSOR_ACCESS   (1ULL << 13)
#define OMNIOS_CAP_NOTIFICATION    (1ULL << 14)
#define OMNIOS_CAP_VIBRATE         (1ULL << 15)
#define OMNIOS_CAP_ADMIN           (1ULL << 63)

/* ── Capability bounds ───────────────────────────────────────────── */

typedef struct {
    uintptr_t start;
    uintptr_t end;
} omnios_cap_bounds_t;

/* ── Token structure ─────────────────────────────────────────────── */

typedef struct {
    uint32_t        id;
    omnios_cap_rights_t rights;
    omnios_cap_bounds_t bounds;
    uint64_t        expiry_ns;
    pid_t           owner_pid;
    char            label[OMNIOS_CAP_LABEL_LEN];
} omnios_cap_token_t;

/* ── Message types ───────────────────────────────────────────────── */

enum omnios_cap_msg_type {
    OMNIOS_CAP_MSG_INVALID      = 0,
    OMNIOS_CAP_MSG_REQUEST      = 1,
    OMNIOS_CAP_MSG_GRANT        = 2,
    OMNIOS_CAP_MSG_DENY         = 3,
    OMNIOS_CAP_MSG_REVOKE       = 4,
    OMNIOS_CAP_MSG_FORWARD      = 5,
    OMNIOS_CAP_MSG_VERIFY       = 6,
    OMNIOS_CAP_MSG_LIST         = 7,
};

/* ── Protocol message ────────────────────────────────────────────── */

typedef struct {
    uint32_t                msg_type;
    uint32_t                seq;
    pid_t                   sender_pid;
    omnios_cap_token_t      token;
    uint32_t                token_count;
    char                    error[128];
} omnios_cap_msg_t;

/* ── Seccomp profile hints (for daemon policy) ───────────────────── */

enum omnios_seccomp_hint {
    OMNIOS_CAP_HINT_STRICT     = 0,
    OMNIOS_CAP_HINT_APP        = 1,
    OMNIOS_CAP_HINT_CONTAINER  = 2,
    OMNIOS_CAP_HINT_COMPOSITOR = 3,
};

/* ── Default policy presets ───────────────────────────────────────── */

static inline omnios_cap_rights_t omnios_cap_default_rights(
        enum omnios_seccomp_hint hint) {
    switch (hint) {
    case OMNIOS_CAP_HINT_STRICT:
        return OMNIOS_CAP_MEMORY_READ | OMNIOS_CAP_MEMORY_WRITE;
    case OMNIOS_CAP_HINT_APP:
        return OMNIOS_CAP_MEMORY_READ | OMNIOS_CAP_MEMORY_WRITE |
               OMNIOS_CAP_FILE_READ | OMNIOS_CAP_FILE_WRITE |
               OMNIOS_CAP_NET_CONNECT | OMNIOS_CAP_NOTIFICATION |
               OMNIOS_CAP_SENSOR_ACCESS;
    case OMNIOS_CAP_HINT_CONTAINER:
        return OMNIOS_CAP_MEMORY_READ | OMNIOS_CAP_MEMORY_WRITE |
               OMNIOS_CAP_MEMORY_EXEC | OMNIOS_CAP_FILE_READ |
               OMNIOS_CAP_FILE_WRITE | OMNIOS_CAP_NET_CONNECT |
               OMNIOS_CAP_NET_BIND | OMNIOS_CAP_IPC_SEND |
               OMNIOS_CAP_IPC_RECV;
    case OMNIOS_CAP_HINT_COMPOSITOR:
        return OMNIOS_CAP_MEMORY_READ | OMNIOS_CAP_MEMORY_WRITE |
               OMNIOS_CAP_DISPLAY_ACCESS | OMNIOS_CAP_INPUT_ACCESS |
               OMNIOS_CAP_IPC_SEND | OMNIOS_CAP_IPC_RECV |
               OMNIOS_CAP_AUDIO_PLAYBACK;
    }
    return 0;
}

#endif /* OMNIOS_CAP_PROTOCOL_H */
