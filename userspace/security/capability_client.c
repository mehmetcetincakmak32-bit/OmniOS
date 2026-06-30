/* OmniOS — userspace/security/capability_client.c */
/* Faz 2 - Capability Token IPC Client Library */
/* SPDX-License-Identifier: MIT */

#define _POSIX_C_SOURCE 200809L
#include "capability_client.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

struct omnios_cap_ctx {
    int fd;
    uint32_t seq;
};

static const char *error_table[] = {
    [0]      = "success",
    [EPERM]  = "permission denied",
    [ENOENT] = "token not found",
    [ENOSPC] = "token store full",
    [ETIME]  = "token expired",
    [EACCES] = "access denied by policy",
};

const char *omnios_cap_strerror(int err) {
    err = -err;
    if (err >= 0 && (size_t)err < sizeof(error_table) / sizeof(error_table[0])
        && error_table[err]) {
        return error_table[err];
    }
    return strerror(err);
}

omnios_cap_ctx_t *omnios_cap_connect(void) {
    omnios_cap_ctx_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return NULL;

    ctx->fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if (ctx->fd < 0) {
        free(ctx);
        return NULL;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, OMNIOS_CAP_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(ctx->fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(ctx->fd);
        free(ctx);
        return NULL;
    }

    ctx->seq = 1;
    return ctx;
}

void omnios_cap_disconnect(omnios_cap_ctx_t *ctx) {
    if (!ctx) return;
    close(ctx->fd);
    free(ctx);
}

static int send_recv(omnios_cap_ctx_t *ctx, omnios_cap_msg_t *msg) {
    msg->seq = ctx->seq++;
    msg->sender_pid = getpid();

    if (send(ctx->fd, msg, sizeof(*msg), MSG_NOSIGNAL) < 0) {
        return -errno;
    }

    omnios_cap_msg_t resp;
    memset(&resp, 0, sizeof(resp));
    ssize_t n = recv(ctx->fd, &resp, sizeof(resp), MSG_NOSIGNAL);
    if (n <= 0) {
        return n == 0 ? -ECONNRESET : -errno;
    }

    if (resp.msg_type == OMNIOS_CAP_MSG_DENY) {
        if (resp.error[0]) {
            fprintf(stderr, "capd: %s\n", resp.error);
        }
        return -EACCES;
    }

    *msg = resp;
    return 0;
}

int omnios_cap_request(omnios_cap_ctx_t *ctx, omnios_cap_rights_t rights,
        const char *label) {
    omnios_cap_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = OMNIOS_CAP_MSG_REQUEST;
    msg.sender_pid = getpid();
    msg.token.rights = rights;
    strncpy(msg.token.label, label ? label : "unnamed",
        OMNIOS_CAP_LABEL_LEN - 1);

    int ret = send_recv(ctx, &msg);
    if (ret < 0) return ret;
    return (int)msg.token.id;
}

int omnios_cap_revoke(omnios_cap_ctx_t *ctx, int token_id) {
    omnios_cap_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = OMNIOS_CAP_MSG_REVOKE;
    msg.token.id = (uint32_t)token_id;
    return send_recv(ctx, &msg);
}

int omnios_cap_verify(omnios_cap_ctx_t *ctx, int token_id,
        omnios_cap_token_t *out) {
    omnios_cap_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = OMNIOS_CAP_MSG_VERIFY;
    msg.token.id = (uint32_t)token_id;

    int ret = send_recv(ctx, &msg);
    if (ret < 0) return ret;
    if (out) *out = msg.token;
    return 0;
}

int omnios_cap_list(omnios_cap_ctx_t *ctx, uint32_t *count_out) {
    omnios_cap_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = OMNIOS_CAP_MSG_LIST;

    int ret = send_recv(ctx, &msg);
    if (ret < 0) return ret;
    if (count_out) *count_out = msg.token_count;
    return 0;
}
