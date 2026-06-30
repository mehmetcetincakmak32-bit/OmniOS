/* OmniOS — kernel/fs/devfs.c */
/* Device filesystem — /dev node creation */
/* SPDX-License-Identifier: MIT */

#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

#define DEVFS_MAX_NODES  64

typedef struct {
    char      name[32];
    uint32_t  dev_id;
    int       type;      /* 0=char, 1=block */
    int       in_use;
} devfs_node_t;

static devfs_node_t _devfs_nodes[DEVFS_MAX_NODES];
static int _devfs_initialized = 0;

void devfs_init(void) {
    memset(_devfs_nodes, 0, sizeof(_devfs_nodes));
    _devfs_initialized = 1;

    /* Create standard nodes */
    devfs_mknod("null",    1, 3, 0);
    devfs_mknod("zero",    1, 5, 0);
    devfs_mknod("console", 1, 5, 0);
    devfs_mknod("tty0",    4, 0, 0);
    devfs_mknod("kmsg",    1, 11, 0);

    printf("[DEVFS] /dev initialized (%d nodes)\n", DEVFS_MAX_NODES);
}

int devfs_mknod(const char *name, uint32_t dev_id, int type, int perms) {
    if (!_devfs_initialized || !name) return -1;
    (void)perms;

    for (int i = 0; i < DEVFS_MAX_NODES; i++) {
        if (!_devfs_nodes[i].in_use) {
            strncpy(_devfs_nodes[i].name, name, sizeof(_devfs_nodes[i].name) - 1);
            _devfs_nodes[i].dev_id = dev_id;
            _devfs_nodes[i].type   = type;
            _devfs_nodes[i].in_use = 1;
            return i;
        }
    }
    return -1;
}

int devfs_lookup(const char *name) {
    if (!_devfs_initialized || !name) return -1;
    for (int i = 0; i < DEVFS_MAX_NODES; i++) {
        if (_devfs_nodes[i].in_use && strcmp(_devfs_nodes[i].name, name) == 0)
            return i;
    }
    return -1;
}

int devfs_get_dev_id(int node) {
    if (node < 0 || node >= DEVFS_MAX_NODES || !_devfs_nodes[node].in_use)
        return -1;
    return (int)_devfs_nodes[node].dev_id;
}

void devfs_list(void) {
    printf("[DEVFS] /dev nodes:\n");
    for (int i = 0; i < DEVFS_MAX_NODES; i++) {
        if (_devfs_nodes[i].in_use) {
            printf("  %s (dev=%u, type=%s)\n",
                _devfs_nodes[i].name,
                _devfs_nodes[i].dev_id,
                _devfs_nodes[i].type == 0 ? "char" : "block");
        }
    }
}
