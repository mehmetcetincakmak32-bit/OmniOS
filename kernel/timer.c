/*
 * OmniOS Timer Subsystem
 * System timing, timer management, uptime tracking
 */

#include "include/omnios_kernel.h"
#include <string.h>
#include <stdio.h>

#define MAX_TIMERS 64

static timer_t _timers[MAX_TIMERS];
static uint32_t _timer_count = 0;
static uint32_t _next_timer_id = 1;
static volatile time_t _uptime_ms = 0;

/* Tick counter from kernel_main */
extern volatile uint64_t _ticks;

status_t timer_init(void) {
    memset(_timers, 0, sizeof(_timers));
    _timer_count = 0;
    _next_timer_id = 1;
    _uptime_ms = 0;
    printf("[TIMER] Zamanlayici servisi hazir (%d timer)\n", MAX_TIMERS);
    return STATUS_SUCCESS;
}

uint32_t timer_create(time_t interval_ms, timer_callback_t callback, void* arg, bool periodic) {
    if (!callback) return 0;
    if (_timer_count >= MAX_TIMERS) return 0;

    timer_t* t = &_timers[_timer_count];
    t->timer_id = _next_timer_id++;
    t->interval_ms = interval_ms;
    t->callback = callback;
    t->arg = arg;
    t->periodic = periodic;
    t->active = true;
    _timer_count++;

    return t->timer_id;
}

status_t timer_cancel(uint32_t timer_id) {
    for (uint32_t i = 0; i < _timer_count; i++) {
        if (_timers[i].timer_id == timer_id && _timers[i].active) {
            _timers[i].active = false;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

time_t timer_get_uptime(void) {
    return _uptime_ms;
}

/* Called every tick */
void timer_tick(void) {
    _uptime_ms++;

    /* Check timers */
    for (uint32_t i = 0; i < _timer_count; i++) {
        if (!_timers[i].active) continue;

        if (_uptime_ms % _timers[i].interval_ms == 0) {
            if (_timers[i].callback)
                _timers[i].callback(_timers[i].arg);

            if (!_timers[i].periodic)
                _timers[i].active = false;
        }
    }
}
