/* OmniOS — userspace/security/waydroid_binder.c */
/* Faz 2 - Waydroid Binder Integration Layer */
/* SPDX-License-Identifier: MIT */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/android/binder.h>

#include "capability_client.h"

/* ── Binder node management ──────────────────────────────────────── */

#define BINDER_DEV       "/dev/binder"
#define BINDER_VND_DEV   "/dev/binder"
#define HWBINDER_DEV     "/dev/hwbinder"
#define VNDBINDER_DEV    "/dev/vndbinder"

struct binder_node {
    int fd;
    char path[64];
};

static struct binder_node binder_nodes[3];
static int binder_node_count = 0;

static int open_binder(const char *path) {
    int fd = open(path, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "Warning: cannot open %s: %s\n",
            path, strerror(errno));
        return -errno;
    }

    /* Check binder version */
    int32_t version = 0;
    if (ioctl(fd, BINDER_VERSION, &version) < 0) {
        fprintf(stderr, "Warning: BINDER_VERSION failed on %s: %s\n",
            path, strerror(errno));
        close(fd);
        return -errno;
    }

    fprintf(stdout, "  %s: binder v%d\n", path, version);

    binder_nodes[binder_node_count].fd = fd;
    strncpy(binder_nodes[binder_node_count].path, path,
        sizeof(binder_nodes[binder_node_count].path) - 1);
    binder_node_count++;
    return fd;
}

/* ── Waydroid container setup ────────────────────────────────────── */

static bool create_device_nodes(void) {
    /* In a real deployment, these would be created by udev or devtmpfs.
     * For QEMU/dev, we try to open them. */

    fprintf(stdout, "[binder] probing binder devices...\n");
    open_binder(BINDER_DEV);
    open_binder(HWBINDER_DEV);
    open_binder(VNDBINDER_DEV);

    if (binder_node_count == 0) {
        fprintf(stderr, "[binder] no binder devices found. "
            "Ensure CONFIG_ANDROID_BINDER_IPC=y in kernel config.\n");
        return false;
    }

    fprintf(stdout, "[binder] %d binder device(s) available\n",
        binder_node_count);
    return true;
}

/* ── Capability token for binder access ──────────────────────────── */

static int register_binder_capability(omnios_cap_ctx_t *cap_ctx) {
    if (!cap_ctx) return 0;

    int tid = omnios_cap_request(cap_ctx,
        OMNIOS_CAP_IPC_SEND | OMNIOS_CAP_IPC_RECV,
        "waydroid-binder");
    if (tid < 0) {
        fprintf(stderr, "[binder] capability request failed: %s\n",
            omnios_cap_strerror(tid));
        return tid;
    }

    fprintf(stdout, "[binder] registered binder capability token=%d\n", tid);
    return tid;
}

/* ── Launch Waydroid container ───────────────────────────────────── */

int omnios_waydroid_start(const char *container_id) {
    (void)container_id;

    fprintf(stdout, "[waydroid] starting Android container...\n");

    /* 1. Connect to capability daemon */
    omnios_cap_ctx_t *cap_ctx = omnios_cap_connect();
    if (!cap_ctx) {
        fprintf(stderr, "[waydroid] cannot connect to capd\n");
    }

    /* 2. Open binder devices */
    if (!create_device_nodes()) {
        fprintf(stderr, "[waydroid] binder setup failed\n");
        if (cap_ctx) omnios_cap_disconnect(cap_ctx);
        return 1;
    }

    /* 3. Register binder capability */
    register_binder_capability(cap_ctx);

    /* 4. Launch waydroid container (via s6-rc or direct) */
    pid_t pid = fork();
    if (pid == 0) {
        /* Child: exec waydroid */
        char *argv[] = {
            "/usr/bin/waydroid",
            "session",
            "start",
            NULL
        };
        execvp(argv[0], argv);
        fprintf(stderr, "[waydroid] exec failed: %s\n", strerror(errno));
        _exit(EXIT_FAILURE);
    }

    if (cap_ctx) omnios_cap_disconnect(cap_ctx);
    fprintf(stdout, "[waydroid] container started (pid=%d)\n", pid);
    return 0;
}

int omnios_waydroid_stop(void) {
    fprintf(stdout, "[waydroid] stopping container...\n");
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = {
            "/usr/bin/waydroid",
            "session",
            "stop",
            NULL
        };
        execvp(argv[0], argv);
        _exit(EXIT_FAILURE);
    }
    return 0;
}

/* ── main (standalone test) ──────────────────────────────────────── */

#ifdef OMNiOS_WAYDROID_TEST
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("=== OmniOS Waydroid Binder Integration ===\n\n");

    if (argc > 1 && strcmp(argv[1], "stop") == 0) {
        return omnios_waydroid_stop();
    }

    return omnios_waydroid_start("default");
}
#endif
