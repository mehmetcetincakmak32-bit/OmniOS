/* OmniOS — kernel/fs/procfs.h */
/* Process filesystem interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_PROCFS_H
#define OMNIOS_PROCFS_H

#include <stddef.h>

void procfs_init(void);
int  procfs_add(const char *name, int (*read_fn)(char *, size_t));
int  procfs_read(const char *name, char *buf, size_t size);
void procfs_list(void);

#endif
