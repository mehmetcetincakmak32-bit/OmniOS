/*
 * OmniOS State Machine
 * Manages system states and UI mode transitions
 */

#include "include/omnios_core.h"
#include <string.h>
#include <stdio.h>

static SystemState _current_state = STATE_BOOTING;
static UIMode _current_mode = MODE_NORMAL;

static const char* _state_names[] = {
    "BOOTING", "IDLE", "RUNNING", "SLEEPING", "SHUTDOWN"
};

static const char* _mode_names[] = {
    "normal", "flow"
};

/* Mode transition matrix: [current][next] -> allowed? */
static const bool _mode_transitions[2][2] = {
    /* TO: NORMAL  FLOW */
    /* FROM NORMAL */ { false, true  },
    /* FROM FLOW    */ { true,  false },
};

/* State transition matrix: [current][next] -> allowed? */
static const bool _state_transitions[5][5] = {
    /* BOOTING IDLE RUNNING SLEEPING SHUTDOWN */
    /* BOOTING    */ { false, true,  false, false, true  },
    /* IDLE       */ { false, false, true,  true,  true  },
    /* RUNNING    */ { false, false, true,  true,  true  },
    /* SLEEPING   */ { false, true,  true,  false, true  },
    /* SHUTDOWN   */ { false, false, false, false, false },
};

void om_system_init(SystemConfig* config) {
    _current_state = STATE_BOOTING;
    _current_mode = MODE_NORMAL;
    _current_state = STATE_IDLE;
}

void om_system_shutdown(void) {
    _current_state = STATE_SHUTDOWN;
}

SystemState om_system_get_state(void) {
    return _current_state;
}

bool om_system_set_state(SystemState state) {
    if (state < STATE_BOOTING || state > STATE_SHUTDOWN)
        return false;

    if (!_state_transitions[_current_state][state])
        return false;

    _current_state = state;
    return true;
}

/* Mode management */
UIMode om_mode_get_current(void) {
    return _current_mode;
}

bool om_mode_set(UIMode mode) {
    if (mode != MODE_NORMAL && mode != MODE_FLOW)
        return false;

    if (!_mode_transitions[_current_mode][mode])
        return false;

    _current_mode = mode;
    return true;
}

UIMode om_mode_toggle(void) {
    UIMode new_mode = (_current_mode == MODE_NORMAL) ? MODE_FLOW : MODE_NORMAL;
    om_mode_set(new_mode);
    return _current_mode;
}

const char* om_mode_get_name(UIMode mode) {
    if (mode >= MODE_NORMAL && mode <= MODE_FLOW)
        return _mode_names[mode];
    return "unknown";
}

/* System info */
uint32_t om_system_get_uptime(void) {
    static uint64_t boot_time = 0;
    if (boot_time == 0) {
        boot_time = 1;
    }
    return (uint32_t)(boot_time);
}

void om_system_get_info(char* buffer, uint32_t buffer_size) {
    if (!buffer || buffer_size < 128) return;

    snprintf(buffer, buffer_size,
        "{\"state\":\"%s\",\"mode\":\"%s\",\"uptime\":%u}",
        _state_names[_current_state],
        _mode_names[_current_mode],
        om_system_get_uptime());
}
