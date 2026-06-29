/*
 * OmniOS Virtual File System
 * Abstraction layer for multiple filesystem types
 */

#include "../include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

#define MAX_FILESYSTEMS    8
#define MAX_MOUNT_POINTS   16

typedef struct filesystem {
    char        name[32];
    status_t (*mount)(const char* source, const char* target);
    status_t (*unmount)(const char* target);
    int      (*open)(const char* path, uint32_t flags);
    status_t (*close)(int fd);
    status_t (*read)(int fd, uint8_t* buffer, uint32_t size, uint32_t* bytes_read);
    status_t (*write)(int fd, const uint8_t* buffer, uint32_t size, uint32_t* bytes_written);
    status_t (*stat)(const char* path, stat_t* st);
    status_t (*mkdir)(const char* path);
    status_t (*getdents)(int fd, dirent_t* dirp, uint32_t count);
} filesystem_t;

typedef struct mount_point {
    char        source[PATH_MAX];
    char        target[PATH_MAX];
    filesystem_t* fs;
    bool        mounted;
} mount_point_t;

static filesystem_t _filesystems[MAX_FILESYSTEMS];
static mount_point_t _mounts[MAX_MOUNT_POINTS];
static uint32_t _fs_count = 0;
static uint32_t _mount_count = 0;

static fd_t _file_table[FILE_MAX_OPEN];
static uint32_t _next_fd = 3;  /* 0=stdin, 1=stdout, 2=stderr */

/* Built-in RAM filesystem */
static status_t _ramfs_mount(const char* source, const char* target);
static status_t _ramfs_unmount(const char* target);
static int      _ramfs_open(const char* path, uint32_t flags);
static status_t _ramfs_read(int fd, uint8_t* buffer, uint32_t size, uint32_t* bytes_read);
static status_t _ramfs_write(int fd, const uint8_t* buffer, uint32_t size, uint32_t* bytes_written);
static status_t _ramfs_stat(const char* path, stat_t* st);
static status_t _ramfs_mkdir(const char* path);

/* Simple RAM-backed node */
typedef struct ram_node {
    char        name[NAME_MAX];
    file_type_t type;
    uint8_t*    data;
    uint32_t    size;
    uint32_t    capacity;
    struct ram_node* parent;
    struct ram_node* children;
    struct ram_node* next;
} ram_node_t;

static ram_node_t* _ram_root = NULL;

status_t vfs_init(void) {
    memset(_filesystems, 0, sizeof(_filesystems));
    memset(_mounts, 0, sizeof(_mounts));
    memset(_file_table, 0, sizeof(_file_table));
    _fs_count = 0;
    _mount_count = 0;
    _next_fd = 3;

    /* Register built-in RAM filesystem */
    filesystem_t ramfs;
    memset(&ramfs, 0, sizeof(ramfs));
    strcpy(ramfs.name, "ramfs");
    ramfs.mount = _ramfs_mount;
    ramfs.unmount = _ramfs_unmount;
    ramfs.open = _ramfs_open;
    ramfs.read = _ramfs_read;
    ramfs.write = _ramfs_write;
    ramfs.stat = _ramfs_stat;
    ramfs.mkdir = _ramfs_mkdir;
    _filesystems[_fs_count++] = ramfs;

    /* Create root node */
    _ram_root = (ram_node_t*)kmalloc(sizeof(ram_node_t));
    if (_ram_root) {
        strcpy(_ram_root->name, "/");
        _ram_root->type = FILE_TYPE_DIRECTORY;
        _ram_root->data = NULL;
        _ram_root->size = 0;
        _ram_root->capacity = 0;
        _ram_root->parent = NULL;
        _ram_root->children = NULL;
        _ram_root->next = NULL;
    }

    /* Mount root */
    _ramfs_mount("ram", "/");

    printf("[VFS] Sanal dosya sistemi hazir (ramfs: /)\n");
    return STATUS_SUCCESS;
}

int vfs_open(const char* path, uint32_t flags) {
    if (!path) return STATUS_INVALID;

    for (uint32_t i = 0; i < _mount_count; i++) {
        if (_mounts[i].mounted && strncmp(path, _mounts[i].target, strlen(_mounts[i].target)) == 0) {
            int fd = _mounts[i].fs->open(path, flags);
            if (fd >= 0) return fd;
        }
    }

    /* Fallback to ramfs */
    return _ramfs_open(path, flags);
}

status_t vfs_close(int fd) {
    if (fd < 0 || fd >= FILE_MAX_OPEN) return STATUS_INVALID;
    if (!_file_table[fd].in_use) return STATUS_INVALID;
    _file_table[fd].in_use = false;
    return STATUS_SUCCESS;
}

status_t vfs_read(int fd, uint8_t* buffer, uint32_t size, uint32_t* bytes_read) {
    if (fd < 0 || fd >= FILE_MAX_OPEN || !_file_table[fd].in_use)
        return STATUS_INVALID;
    return _ramfs_read(fd, buffer, size, bytes_read);
}

status_t vfs_write(int fd, const uint8_t* buffer, uint32_t size, uint32_t* bytes_written) {
    if (fd < 0 || fd >= FILE_MAX_OPEN || !_file_table[fd].in_use)
        return STATUS_INVALID;
    return _ramfs_write(fd, buffer, size, bytes_written);
}

status_t vfs_stat(const char* path, stat_t* st) {
    return _ramfs_stat(path, st);
}

status_t vfs_mkdir(const char* path) {
    return _ramfs_mkdir(path);
}

/* ================================================================
 * RAM Filesystem Implementation
 * ================================================================ */

static ram_node_t* _ram_find_node(const char* path) {
    if (!path || !_ram_root) return NULL;
    if (strcmp(path, "/") == 0) return _ram_root;

    char copy[PATH_MAX];
    strncpy(copy, path, sizeof(copy) - 1);
    char* save;
    char* token = strtok_r(copy, "/", &save);

    ram_node_t* current = _ram_root;
    while (token && current) {
        ram_node_t* child = current->children;
        while (child) {
            if (strcmp(child->name, token) == 0) {
                current = child;
                break;
            }
            child = child->next;
        }
        if (!child) return NULL;
        token = strtok_r(NULL, "/", &save);
    }
    return current;
}

static status_t _ramfs_mount(const char* source, const char* target) {
    if (_mount_count >= MAX_MOUNT_POINTS) return STATUS_NO_MEMORY;
    strncpy(_mounts[_mount_count].source, source ? source : "ram", PATH_MAX - 1);
    strncpy(_mounts[_mount_count].target, target, PATH_MAX - 1);
    _mounts[_mount_count].fs = &_filesystems[0];
    _mounts[_mount_count].mounted = true;
    _mount_count++;
    return STATUS_SUCCESS;
}

static status_t _ramfs_unmount(const char* target) {
    for (uint32_t i = 0; i < _mount_count; i++) {
        if (_mounts[i].mounted && strcmp(_mounts[i].target, target) == 0) {
            _mounts[i].mounted = false;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

static int _ramfs_open(const char* path, uint32_t flags) {
    ram_node_t* node = _ram_find_node(path);
    if (!node) return STATUS_NOT_FOUND;

    int fd = _next_fd++;
    if (fd >= FILE_MAX_OPEN) return STATUS_NO_MEMORY;

    _file_table[fd].fd = fd;
    _file_table[fd].type = node->type;
    _file_table[fd].pos = 0;
    _file_table[fd].flags = flags;
    _file_table[fd].private_data = node;
    _file_table[fd].in_use = true;

    return fd;
}

static status_t _ramfs_read(int fd, uint8_t* buffer, uint32_t size, uint32_t* bytes_read) {
    if (!_file_table[fd].in_use) return STATUS_INVALID;
    ram_node_t* node = (ram_node_t*)_file_table[fd].private_data;
    if (!node || !node->data) return STATUS_ERROR;

    uint32_t available = node->size - _file_table[fd].pos;
    uint32_t to_read = size < available ? size : available;

    if (buffer && to_read > 0) {
        memcpy(buffer, node->data + _file_table[fd].pos, to_read);
        _file_table[fd].pos += to_read;
    }

    if (bytes_read) *bytes_read = to_read;
    return STATUS_SUCCESS;
}

static status_t _ramfs_write(int fd, const uint8_t* buffer, uint32_t size, uint32_t* bytes_written) {
    if (!_file_table[fd].in_use) return STATUS_INVALID;
    ram_node_t* node = (ram_node_t*)_file_table[fd].private_data;
    if (!node) return STATUS_ERROR;

    uint32_t needed = _file_table[fd].pos + size;
    if (needed > node->capacity) {
        uint32_t new_cap = needed + 1024;
        uint8_t* new_data = (uint8_t*)krealloc(node->data, new_cap);
        if (!new_data) return STATUS_NO_MEMORY;
        node->data = new_data;
        node->capacity = new_cap;
    }

    if (buffer && size > 0) {
        memcpy(node->data + _file_table[fd].pos, buffer, size);
        _file_table[fd].pos += size;
        if (_file_table[fd].pos > node->size)
            node->size = _file_table[fd].pos;
    }

    if (bytes_written) *bytes_written = size;
    return STATUS_SUCCESS;
}

static status_t _ramfs_stat(const char* path, stat_t* st) {
    ram_node_t* node = _ram_find_node(path);
    if (!node) return STATUS_NOT_FOUND;
    if (!st) return STATUS_INVALID;

    memset(st, 0, sizeof(stat_t));
    st->st_size = node->size;
    st->st_mode = (node->type == FILE_TYPE_DIRECTORY) ? 040755 : 0100644;
    st->st_nlink = 1;
    return STATUS_SUCCESS;
}

static status_t _ramfs_mkdir(const char* path) {
    if (!path || !_ram_root) return STATUS_INVALID;

    char parent_path[PATH_MAX];
    char dir_name[NAME_MAX];
    strncpy(parent_path, path, sizeof(parent_path) - 1);

    char* last_slash = strrchr(parent_path, '/');
    if (!last_slash) return STATUS_INVALID;

    strncpy(dir_name, last_slash + 1, sizeof(dir_name) - 1);
    *last_slash = '\0';

    ram_node_t* parent = _ram_find_node(parent_path);
    if (!parent || parent->type != FILE_TYPE_DIRECTORY) return STATUS_NOT_FOUND;

    ram_node_t* node = (ram_node_t*)kmalloc(sizeof(ram_node_t));
    if (!node) return STATUS_NO_MEMORY;

    strncpy(node->name, dir_name, NAME_MAX - 1);
    node->type = FILE_TYPE_DIRECTORY;
    node->data = NULL;
    node->size = 0;
    node->capacity = 0;
    node->parent = parent;
    node->children = NULL;
    node->next = parent->children;
    parent->children = node;

    return STATUS_SUCCESS;
}
