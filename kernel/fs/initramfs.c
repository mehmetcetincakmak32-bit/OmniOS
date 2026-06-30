/* OmniOS — kernel/fs/initramfs.c */
/* cpio initramfs — embedded archive loader */
/* SPDX-License-Identifier: MIT */

#include "../include/omnios_kernel.h"
#include <stdio.h>
#include <string.h>

#define CPIO_MAGIC       "070701"
#define CPIO_HEADER_LEN  110
#define PATH_MAX         4096

typedef struct {
    char    c_magic[6];
    char    c_ino[8];
    char    c_mode[8];
    char    c_uid[8];
    char    c_gid[8];
    char    c_nlink[8];
    char    c_mtime[8];
    char    c_filesize[8];
    char    c_devmajor[8];
    char    c_devminor[8];
    char    c_rdevmajor[8];
    char    c_rdevminor[8];
    char    c_namesize[8];
    char    c_check[8];
} __attribute__((packed)) cpio_header_t;

static unsigned long parse_hex(const char *s, int len) {
    unsigned long val = 0;
    for (int i = 0; i < len && s[i]; i++) {
        char c = s[i];
        val <<= 4;
        if (c >= '0' && c <= '9')      val |= c - '0';
        else if (c >= 'a' && c <= 'f') val |= c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val |= c - 'A' + 10;
    }
    return val;
}

typedef struct {
    const uint8_t *data;
    size_t         size;
} initramfs_entry_t;

typedef struct {
    const void *archive;
    size_t      archive_size;
    size_t      offset;
} initramfs_t;

void initramfs_init(initramfs_t *fs, const void *archive, size_t size) {
    fs->archive = archive;
    fs->archive_size = size;
    fs->offset = 0;
}

int initramfs_next(initramfs_t *fs, char *name, size_t namelen, initramfs_entry_t *entry) {
    if (fs->offset >= fs->archive_size) return 0;

    const uint8_t *p = (const uint8_t *)fs->archive + fs->offset;
    if (fs->offset + CPIO_HEADER_LEN > fs->archive_size) return -1;

    const cpio_header_t *hdr = (const cpio_header_t *)p;

    if (memcmp(hdr->c_magic, CPIO_MAGIC, 6) != 0) return 0;

    unsigned long namesize = parse_hex(hdr->c_namesize, 8);
    unsigned long filesize = parse_hex(hdr->c_filesize, 8);

    /* Skip header */
    size_t name_off = fs->offset + CPIO_HEADER_LEN;
    size_t data_off = name_off + namesize;
    data_off = (data_off + 3) & ~3;  /* Align to 4 */

    if (name_off + namesize > fs->archive_size || data_off + filesize > fs->archive_size)
        return -1;

    if (name) {
        size_t cplen = namesize < namelen ? namesize : namelen - 1;
        memcpy(name, (const char *)fs->archive + name_off, cplen);
        name[cplen] = '\0';
    }

    if (entry) {
        entry->data = (const uint8_t *)fs->archive + data_off;
        entry->size = filesize;
    }

    /* Advance to next entry */
    fs->offset = data_off + filesize;
    fs->offset = (fs->offset + 3) & ~3;

    return 1;
}

int initramfs_find(initramfs_t *fs, const char *target, initramfs_entry_t *entry) {
    size_t saved = fs->offset;
    fs->offset = 0;

    char name[PATH_MAX];
    while (1) {
        int ret = initramfs_next(fs, name, sizeof(name), entry);
        if (ret <= 0) break;
        if (strcmp(name, target) == 0) return 1;
    }

    fs->offset = saved;
    return 0;
}
