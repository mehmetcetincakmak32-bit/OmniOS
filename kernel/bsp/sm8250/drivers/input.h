/* OmniOS — kernel/bsp/sm8250/drivers/input.h */
/* Input subsystem interface */
/* SPDX-License-Identifier: MIT */

#ifndef OMNIOS_INPUT_H
#define OMNIOS_INPUT_H

#include <stdint.h>

typedef struct {
    uint32_t type;
    uint32_t code;
    int32_t  x;
    int32_t  y;
    int32_t  value;
    uint64_t timestamp;
} input_event_t;

enum {
    INPUT_TOUCH_DOWN  = 1,
    INPUT_TOUCH_MOVE  = 2,
    INPUT_TOUCH_UP    = 3,
    INPUT_KEY_DOWN    = 4,
    INPUT_KEY_UP      = 5,
    INPUT_PROXIMITY   = 6,
    INPUT_ORIENTATION = 7,
};

int  input_init(void);
int  input_read_event(input_event_t *ev);
void input_touch_event(int type, int x, int y);
void input_key_event(int keycode, int down);
void input_check_buttons(void);
int  input_poll(int *fd);

#endif
