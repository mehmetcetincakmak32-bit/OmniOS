/* OmniOS — kernel/ipc/pipe.h */
/* Pipe IPC interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_PIPE_H
#define OMNIOS_PIPE_H

#include <stddef.h>
#include <stdint.h>

void pipe_init(void);
int  pipe_create(int *read_fd, int *write_fd);
int  pipe_write(int pipe_id, const uint8_t *data, size_t len);
int  pipe_read(int pipe_id, uint8_t *buf, size_t maxlen);
int  pipe_close(int pipe_id, int is_write);

#endif
