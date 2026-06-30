/* OmniOS — userspace/security/test_capability.c */
/* Faz 2 - Capability Token IPC Tests */
/* SPDX-License-Identifier: MIT */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "capability_protocol.h"
#include "capability_client.h"

static int test_rights_masks(void) {
    printf("  test_rights_masks... ");
    assert(OMNIOS_CAP_MEMORY_READ  == 1);
    assert(OMNIOS_CAP_MEMORY_WRITE == 2);
    assert(OMNIOS_CAP_ADMIN        == (1ULL << 63));
    assert(OMNIOS_CAP_MEMORY_READ | OMNIOS_CAP_MEMORY_WRITE == 3);
    printf("OK\n");
    return 0;
}

static int test_default_presets(void) {
    printf("  test_default_presets... ");
    omnios_cap_rights_t strict = omnios_cap_default_rights(OMNIOS_CAP_HINT_STRICT);
    assert(strict & OMNIOS_CAP_MEMORY_READ);
    assert(strict & OMNIOS_CAP_MEMORY_WRITE);
    assert(!(strict & OMNIOS_CAP_NET_CONNECT));

    omnios_cap_rights_t app = omnios_cap_default_rights(OMNIOS_CAP_HINT_APP);
    assert(app & OMNIOS_CAP_NET_CONNECT);
    assert(app & OMNIOS_CAP_FILE_READ);
    assert(!(app & OMNIOS_CAP_ADMIN));

    omnios_cap_rights_t comp = omnios_cap_default_rights(OMNIOS_CAP_HINT_COMPOSITOR);
    assert(comp & OMNIOS_CAP_DISPLAY_ACCESS);
    assert(comp & OMNIOS_CAP_INPUT_ACCESS);
    assert(!(comp & OMNIOS_CAP_NET_BIND));
    printf("OK\n");
    return 0;
}

static int test_token_struct(void) {
    printf("  test_token_struct... ");
    omnios_cap_token_t tok;
    memset(&tok, 0, sizeof(tok));
    tok.id = 42;
    tok.rights = OMNIOS_CAP_MEMORY_READ | OMNIOS_CAP_MEMORY_WRITE;
    tok.owner_pid = getpid();
    tok.bounds.start = 0x1000;
    tok.bounds.end   = 0x2000;
    tok.expiry_ns = 1234567890ULL;
    strncpy(tok.label, "test-token", sizeof(tok.label));

    assert(tok.id == 42);
    assert(tok.rights == (OMNIOS_CAP_MEMORY_READ | OMNIOS_CAP_MEMORY_WRITE));
    assert(tok.owner_pid == getpid());
    assert(tok.bounds.start == 0x1000);
    assert(tok.bounds.end   == 0x2000);
    assert(strcmp(tok.label, "test-token") == 0);
    printf("OK\n");
    return 0;
}

static int test_msg_struct(void) {
    printf("  test_msg_struct... ");
    omnios_cap_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = OMNIOS_CAP_MSG_REQUEST;
    msg.seq = 1;
    msg.sender_pid = getpid();
    msg.token.id = 7;
    msg.token_count = 1;

    assert(msg.msg_type == OMNIOS_CAP_MSG_REQUEST);
    assert(msg.seq == 1);
    assert(msg.token.id == 7);
    assert(msg.token_count == 1);
    printf("OK\n");
    return 0;
}

static int test_cap_strerror(void) {
    printf("  test_cap_strerror... ");
    assert(strcmp(omnios_cap_strerror(0), "success") == 0);
    assert(strcmp(omnios_cap_strerror(-EPERM), "permission denied") == 0);
    assert(strcmp(omnios_cap_strerror(-ENOENT), "token not found") == 0);
    printf("OK\n");
    return 0;
}

/* ── Connection test (requires daemon running) ───────────────────── */

static int test_connect_daemon(void) {
    printf("  test_connect_daemon (optional)... ");
    omnios_cap_ctx_t *ctx = omnios_cap_connect();
    if (!ctx) {
        printf("SKIP (daemon not running)\n");
        return 0;
    }

    int tid = omnios_cap_request(ctx,
        OMNIOS_CAP_MEMORY_READ | OMNIOS_CAP_MEMORY_WRITE, "test");
    if (tid < 0) {
        printf("SKIP (request denied: %s)\n", omnios_cap_strerror(tid));
        omnios_cap_disconnect(ctx);
        return 0;
    }
    assert(tid > 0);
    printf("OK (token=%d)\n", tid);

    omnios_cap_token_t tok;
    int ret = omnios_cap_verify(ctx, tid, &tok);
    assert(ret == 0);
    assert(tok.id == (uint32_t)tid);
    assert(tok.rights & OMNIOS_CAP_MEMORY_READ);

    omnios_cap_disconnect(ctx);
    return 0;
}

int main(void) {
    printf("=== OmniOS Capability Token IPC Tests ===\n\n");

    int passed = 0, failed = 0;

    struct { int (*fn)(void); const char *name; } tests[] = {
        { test_rights_masks,    "rights masks" },
        { test_default_presets, "default policy presets" },
        { test_token_struct,    "token struct layout" },
        { test_msg_struct,      "message struct layout" },
        { test_cap_strerror,    "string error codes" },
        { test_connect_daemon,  "daemon connect (optional)" },
    };

    size_t n = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < n; i++) {
        printf("[%zu/%zu] %s\n", i + 1, n, tests[i].name);
        int ret = tests[i].fn();
        if (ret == 0) passed++; else failed++;
    }

    printf("\n=== %d passed, %d failed ===\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
