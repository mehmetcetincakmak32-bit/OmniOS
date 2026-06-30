/* OmniOS — kernel/ipc/pipe.c */
/* Pipe IPC — unidirectional byte stream */
/* SPDX-License-Identifier: MIT */

#include "../include/omnios_kernel.h"
#include "../proc/process.h"
#include <stdio.h>
#include <string.h>

#define PIPE_BUF_SIZE  4096

typedef struct {
    uint8_t  buf[PIPE_BUF_SIZE];
    size_t   head;
    size_t   tail;
    int      read_open;
    int      write_open;
    int      in_use;
} pipe_t;

static pipe_t _pipes[MAX_PIPES];

void pipe_init(void) {
    memset(_pipes, 0, sizeof(_pipes));
    printf("[PIPE] Pipe subsystem initialized (%d pipes)\n", MAX_PIPES);
}

int pipe_create(int *read_fd, int *write_fd) {
    if (!read_fd || !write_fd) return -1;

    int pipe_id = -1;
    for (int i = 0; i < MAX_PIPES; i++) {
        if (!_pipes[i].in_use) { pipe_id = i; break; }
    }
    if (pipe_id < 0) return -1;

    memset(&_pipes[pipe_id], 0, sizeof(pipe_t));
    _pipes[pipe_id].in_use = 1;
    _pipes[pipe_id].read_open = 1;
    _pipes[pipe_id].write_open = 1;

    process_t *proc = proc_get_current();
    if (!proc) return -1;

    *read_fd = -1;
    *write_fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!proc->fds[i].in_use) {
            if (*read_fd < 0) {
                proc->fds[i].fd = i;
                proc->fds[i].type = 1;
                proc->fds[i].pipe_id = pipe_id;
                proc->fds[i].flags = 0;
                proc->fds[i].in_use = 1;
                *read_fd = i;
            } else if (*write_fd < 0) {
                proc->fds[i].fd = i;
                proc->fds[i].type = 1;
                proc->fds[i].pipe_id = pipe_id;
                proc->fds[i].flags = 1;
                proc->fds[i].in_use = 1;
                *write_fd = i;
            } else break;
        }
    }

    return 0;
}

int pipe_write(int pipe_id, const uint8_t *data, size_t len) {
    if (pipe_id < 0 || pipe_id >= MAX_PIPES || !_pipes[pipe_id].in_use)
        return -1;
    if (!_pipes[pipe_id].write_open) return -1;

    pipe_t *pipe = &_pipes[pipe_id];
    size_t written = 0;

    while (written < len) {
        size_t next = (pipe->head + 1) % PIPE_BUF_SIZE;
        if (next == pipe->tail) break;  /* Buffer full */
        pipe->buf[pipe->head] = data[written++];
        pipe->head = next;
    }

    return (int)written;
}

int pipe_read(int pipe_id, uint8_t *buf, size_t maxlen) {
    if (pipe_id < 0 || pipe_id >= MAX_PIPES || !_pipes[pipe_id].in_use)
        return -1;
    if (!_pipes[pipe_id].read_open) return -1;

    pipe_t *pipe = &_pipes[pipe_id];
    size_t read_count = 0;

    while (read_count < maxlen && pipe->tail != pipe->head) {
        buf[read_count++] = pipe->buf[pipe->tail];
        pipe->tail = (pipe->tail + 1) % PIPE_BUF_SIZE;
    }

    return (int)read_count;
}

int pipe_close(int pipe_id, int is_write) {
    if (pipe_id < 0 || pipe_id >= MAX_PIPES || !_pipes[pipe_id].in_use)
        return -1;

    if (is_write)
        _pipes[pipe_id].write_open = 0;
    else
        _pipes[pipe_id].read_open = 0;

    if (!_pipes[pipe_id].read_open && !_pipes[pipe_id].write_open)
        _pipes[pipe_id].in_use = 0;

    return 0;
}
