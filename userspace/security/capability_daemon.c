/* OmniOS — userspace/security/capability_daemon.c */
/* Faz 2 - Capability Token IPC Daemon */
/* SPDX-License-Identifier: MIT */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "capability_protocol.h"

/* ── Internal token store ────────────────────────────────────────── */

static omnios_cap_token_t token_store[OMNIOS_CAP_TOKEN_MAX];
static int token_count = 0;
static int next_token_id = 1;

static uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/* ── Policy engine ───────────────────────────────────────────────── */

struct policy_entry {
    pid_t pid;
    enum omnios_seccomp_hint hint;
    omnios_cap_rights_t max_rights;
};

#define MAX_POLICY 128
static struct policy_entry policy_table[MAX_POLICY];
static int policy_count = 0;

static int policy_lookup(pid_t pid) {
    for (int i = 0; i < policy_count; i++) {
        if (policy_table[i].pid == pid) return i;
    }
    return -1;
}

static int policy_register(pid_t pid, enum omnios_seccomp_hint hint) {
    int idx = policy_lookup(pid);
    if (idx >= 0) return idx;

    if (policy_count >= MAX_POLICY) return -ENOSPC;
    int i = policy_count++;
    policy_table[i].pid = pid;
    policy_table[i].hint = hint;
    policy_table[i].max_rights = omnios_cap_default_rights(hint);
    return i;
}

static bool policy_allows(pid_t pid, omnios_cap_rights_t requested) {
    int idx = policy_lookup(pid);
    if (idx < 0) return false;
    return (policy_table[idx].max_rights & requested) == requested;
}

/* ── Token operations ────────────────────────────────────────────── */

static int token_grant(pid_t owner, omnios_cap_rights_t rights,
        const char *label) {
    if (token_count >= OMNIOS_CAP_TOKEN_MAX) return -ENOSPC;
    if (!policy_allows(owner, rights)) return -EPERM;

    int i = token_count++;
    token_store[i].id = next_token_id++;
    token_store[i].rights = rights;
    token_store[i].bounds.start = 0;
    token_store[i].bounds.end = 0;
    token_store[i].expiry_ns = now_ns() + 3600ULL * 1000000000ULL;
    token_store[i].owner_pid = owner;
    strncpy(token_store[i].label, label, OMNIOS_CAP_LABEL_LEN - 1);
    return token_store[i].id;
}

static omnios_cap_token_t *token_find(int id) {
    for (int i = 0; i < token_count; i++) {
        if (token_store[i].id == (uint32_t)id) return &token_store[i];
    }
    return NULL;
}

static int token_revoke(int id) {
    for (int i = 0; i < token_count; i++) {
        if (token_store[i].id == (uint32_t)id) {
            token_store[i] = token_store[--token_count];
            return 0;
        }
    }
    return -ENOENT;
}

static int token_verify(int id, pid_t requester) {
    omnios_cap_token_t *t = token_find(id);
    if (!t) return -ENOENT;
    if (t->owner_pid != requester) return -EPERM;
    if (now_ns() > t->expiry_ns) return -ETIME;
    return 0;
}

/* ── Handle client message ───────────────────────────────────────── */

static int handle_message(int client_fd, const omnios_cap_msg_t *req) {
    omnios_cap_msg_t resp;
    memset(&resp, 0, sizeof(resp));
    resp.msg_type = OMNIOS_CAP_MSG_DENY;
    resp.seq = req->seq;

    switch (req->msg_type) {
    case OMNIOS_CAP_MSG_REQUEST: {
        int policy_idx = policy_lookup(req->sender_pid);
        if (policy_idx < 0) {
            snprintf(resp.error, sizeof(resp.error),
                "unknown pid %d", req->sender_pid);
            break;
        }
        int tid = token_grant(req->sender_pid, req->token.rights,
            req->token.label);
        if (tid < 0) {
            snprintf(resp.error, sizeof(resp.error),
                "grant failed: %s", strerror(-tid));
            break;
        }
        resp.msg_type = OMNIOS_CAP_MSG_GRANT;
        resp.token.id = tid;
        resp.token.rights = token_find(tid)->rights;
        break;
    }

    case OMNIOS_CAP_MSG_REVOKE: {
        int ret = token_revoke(req->token.id);
        if (ret < 0) {
            snprintf(resp.error, sizeof(resp.error),
                "revoke failed: %s", strerror(-ret));
            break;
        }
        resp.msg_type = OMNIOS_CAP_MSG_GRANT;
        break;
    }

    case OMNIOS_CAP_MSG_VERIFY: {
        int ret = token_verify(req->token.id, req->sender_pid);
        if (ret < 0) {
            snprintf(resp.error, sizeof(resp.error),
                "verify failed: %s", strerror(-ret));
            break;
        }
        resp.msg_type = OMNIOS_CAP_MSG_GRANT;
        resp.token = *token_find(req->token.id);
        break;
    }

    case OMNIOS_CAP_MSG_LIST: {
        resp.token_count = 0;
        for (int i = 0; i < token_count; i++) {
            if (token_store[i].owner_pid == req->sender_pid) {
                resp.token_count++;
            }
        }
        resp.msg_type = OMNIOS_CAP_MSG_GRANT;
        break;
    }

    default:
        snprintf(resp.error, sizeof(resp.error), "unknown msg_type %u",
            req->msg_type);
        break;
    }

    if (send(client_fd, &resp, sizeof(resp), MSG_NOSIGNAL) < 0) {
        return -errno;
    }
    return 0;
}

/* ── Main loop ───────────────────────────────────────────────────── */

static volatile bool running = true;

static void handle_signal(int sig) {
    (void)sig;
    running = false;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    /* Remove stale socket */
    unlink(OMNIOS_CAP_SOCK_PATH);

    int listen_fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, OMNIOS_CAP_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    chmod(OMNIOS_CAP_SOCK_PATH, 0666);

    if (listen(listen_fd, 8) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    fprintf(stdout, "OmniOS capability daemon started (%s)\n",
        OMNIOS_CAP_SOCK_PATH);

    /* Register built-in policies */
    policy_register(1, OMNIOS_CAP_HINT_COMPOSITOR);   /* init/system */
    policy_register(2, OMNIOS_CAP_HINT_STRICT);        /* kernel helper */

    struct pollfd fds[64];
    int nfds = 1;
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    while (running) {
        int ret = poll(fds, nfds, 1000);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("poll");
            break;
        }

        /* Accept new connections */
        if (fds[0].revents & POLLIN) {
            int client_fd = accept4(listen_fd, NULL, NULL,
                SOCK_CLOEXEC);
            if (client_fd >= 0 && nfds < 64) {
                /* Get peer credentials for policy lookup */
                struct ucred cred;
                socklen_t len = sizeof(cred);
                if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED,
                        &cred, &len) == 0) {
                    policy_register(cred.pid, OMNIOS_CAP_HINT_APP);
                }
                fds[nfds].fd = client_fd;
                fds[nfds].events = POLLIN;
                nfds++;
            }
        }

        /* Handle client messages */
        for (int i = 1; i < nfds; i++) {
            if (!(fds[i].revents & POLLIN)) continue;

            omnios_cap_msg_t msg;
            ssize_t n = recv(fds[i].fd, &msg, sizeof(msg), MSG_NOSIGNAL);
            if (n <= 0) {
                close(fds[i].fd);
                fds[i] = fds[--nfds];
                i--;
                continue;
            }
            handle_message(fds[i].fd, &msg);
        }
    }

    fprintf(stdout, "OmniOS capability daemon shutting down\n");

    for (int i = 0; i < nfds; i++) close(fds[i].fd);
    unlink(OMNIOS_CAP_SOCK_PATH);
    return 0;
}
