/*
 * OmniOS IPC Subsystem
 * Inter-Process Communication: channels, pipes, shared memory
 */

#include "../include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

static ipc_channel_t _channels[IPC_MAX_CHANNELS];
static uint32_t _channel_count = 0;
static uint32_t _next_channel_id = 1;

status_t ipc_init(void) {
    memset(_channels, 0, sizeof(_channels));
    _channel_count = 0;
    _next_channel_id = 1;
    printf("[IPC] Inter-Process Communication hazir (%d kanal)\n", IPC_MAX_CHANNELS);
    return STATUS_SUCCESS;
}

status_t ipc_channel_create(ipc_channel_t* channel) {
    if (!channel) return STATUS_INVALID;
    if (_channel_count >= IPC_MAX_CHANNELS) return STATUS_NO_MEMORY;

    uint32_t id = _next_channel_id++;
    _channels[_channel_count].channel_id = id;
    _channels[_channel_count].type = channel->type;
    _channels[_channel_count].owner = channel->owner;
    _channels[_channel_count].peer = channel->peer;
    _channels[_channel_count].pending_count = 0;
    _channels[_channel_count].blocked = false;

    channel->channel_id = id;
    _channel_count++;

    return STATUS_SUCCESS;
}

status_t ipc_channel_close(uint32_t channel_id) {
    for (uint32_t i = 0; i < _channel_count; i++) {
        if (_channels[i].channel_id == channel_id) {
            _channels[i] = _channels[_channel_count - 1];
            _channel_count--;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

status_t ipc_send(uint32_t channel_id, const ipc_message_t* msg) {
    ipc_channel_t* ch = NULL;
    for (uint32_t i = 0; i < _channel_count; i++) {
        if (_channels[i].channel_id == channel_id) {
            ch = &_channels[i];
            break;
        }
    }
    if (!ch) return STATUS_NOT_FOUND;

    ch->pending_count++;
    if (ch->blocked) {
        ch->blocked = false;
        sched_wake(ch->peer);
    }
    return STATUS_SUCCESS;
}

status_t ipc_receive(uint32_t channel_id, ipc_message_t* msg, time_t timeout) {
    ipc_channel_t* ch = NULL;
    for (uint32_t i = 0; i < _channel_count; i++) {
        if (_channels[i].channel_id == channel_id) {
            ch = &_channels[i];
            break;
        }
    }
    if (!ch) return STATUS_NOT_FOUND;

    if (ch->pending_count == 0) {
        if (timeout == 0) return STATUS_WOULD_BLOCK;
        ch->blocked = true;
        sched_sleep(timeout);
        if (ch->pending_count == 0) return STATUS_TIMEOUT;
    }

    ch->pending_count--;
    return STATUS_SUCCESS;
}
