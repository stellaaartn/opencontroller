#pragma once

#include <stdint.h>

typedef struct {
    volatile int32_t  ticks;         // cumulative position counter
    volatile int      direction;     // +1 forward, -1 reverse, 0 unknown
    volatile uint64_t last_tick_us;  // timestamp of last tick (microseconds)
    volatile int      stalled;       // 1 if no tick received within stall timeout
} hall_state_t;

#define STALL_TIMEOUT_US  300000   // 300ms no tick = stalled

void hall_init(void);              // configure GPIO interrupts
void hall_reset(int motor);        // zero the tick counter for a motor
hall_state_t hall_get(int motor);  // snapshot of current state
int  hall_speed_rpm(int motor);    // estimated RPM from tick interval
void hall_check_stall(int motor);  // call periodically; sets stalled flag
