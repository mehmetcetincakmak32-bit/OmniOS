/* OmniOS — kernel/bsp/sm8250/drivers/input.c */
/* Input subsystem — touch, buttons, sensors → event pipe */
/* SPDX-License-Identifier: MIT */

#include "../sm8250.h"
#include <stdio.h>
#include <string.h>

#define MAX_EVENTS        256
#define EVENT_QUEUE_BASE  0x80000000  /* MMIO ring buffer for userspace */

enum input_type {
    INPUT_TOUCH_DOWN  = 1,
    INPUT_TOUCH_MOVE  = 2,
    INPUT_TOUCH_UP    = 3,
    INPUT_KEY_DOWN    = 4,
    INPUT_KEY_UP      = 5,
    INPUT_PROXIMITY   = 6,
    INPUT_ORIENTATION = 7,
};

typedef struct {
    uint32_t type;
    uint32_t code;
    int32_t  x;
    int32_t  y;
    int32_t  value;
    uint64_t timestamp;
} input_event_t;

static input_event_t _event_queue[MAX_EVENTS];
static int _event_head = 0;
static int _event_tail = 0;
static int _input_ready = 0;

int input_init(void) {
    printf("[INPUT] Input subsystem init\n");
    _input_ready = 1;
    return 0;
}

static void push_event(input_event_t *ev) {
    int next = (_event_head + 1) % MAX_EVENTS;
    if (next == _event_tail) return;
    _event_queue[_event_head] = *ev;
    _event_head = next;
}

int input_read_event(input_event_t *ev) {
    if (_event_tail == _event_head) return 0;
    *ev = _event_queue[_event_tail];
    _event_tail = (_event_tail + 1) % MAX_EVENTS;
    return 1;
}

void input_touch_event(int type, int x, int y) {
    if (!_input_ready) return;
    input_event_t ev = {
        .type = (uint32_t)type,
        .code = 0,
        .x = x,
        .y = y,
        .value = (type == INPUT_TOUCH_DOWN) ? 1 : (type == INPUT_TOUCH_UP) ? 0 : 0,
        .timestamp = 0,
    };
    push_event(&ev);
}

void input_key_event(int keycode, int down) {
    if (!_input_ready) return;
    input_event_t ev = {
        .type = down ? INPUT_KEY_DOWN : INPUT_KEY_UP,
        .code = (uint32_t)keycode,
        .x = 0, .y = 0, .value = down ? 1 : 0,
        .timestamp = 0,
    };
    push_event(&ev);
}

/* ── Power / Volume buttons via PMIC GPIO ─────────────────────────── */

void input_check_buttons(void) {
    uint32_t pmic_sts = read_reg(SM8250_PMIC_BASE, 0x4400);
    if (pmic_sts & 0x01) input_key_event(116, 1);  /* KEY_POWER */
    if (pmic_sts & 0x02) input_key_event(114, 1);  /* KEY_VOLUMEDOWN */
    if (pmic_sts & 0x04) input_key_event(115, 1);  /* KEY_VOLUMEUP */
}

int input_poll(int *fd) {
    (void)fd;
    input_check_buttons();
    return _event_head != _event_tail;
}
