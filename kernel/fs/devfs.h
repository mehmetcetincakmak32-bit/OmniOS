/* OmniOS — kernel/fs/devfs.h */
/* Device filesystem interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_DEVFS_H
#define OMNIOS_DEVFS_H

#include <stdint.h>

void devfs_init(void);
int  devfs_mknod(const char *name, uint32_t dev_id, int type, int perms);
int  devfs_lookup(const char *name);
int  devfs_get_dev_id(int node);
void devfs_list(void);

#endif
