/* OmniOS — kernel/fs/initramfs.h */
/* cpio initramfs interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_INITRAMFS_H
#define OMNIOS_INITRAMFS_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *data;
    size_t         size;
} initramfs_entry_t;

typedef struct {
    const void *archive;
    size_t      archive_size;
    size_t      offset;
} initramfs_t;

void initramfs_init(initramfs_t *fs, const void *archive, size_t size);
int  initramfs_next(initramfs_t *fs, char *name, size_t namelen, initramfs_entry_t *entry);
int  initramfs_find(initramfs_t *fs, const char *target, initramfs_entry_t *entry);

#endif
