#pragma once

#include <stdint.h>

// Max tick difference allowed before anti-tilt triggers
#define SYNC_TILT_THRESHOLD   5

// How much to throttle the faster motor when out of sync (duty units)
#define SYNC_THROTTLE_STEP    10

typedef enum {
    SYNC_OK,
    SYNC_CORRECTING,   // auto-correcting after tilt
    SYNC_FAULT,        // fault state, requires explicit clear
} sync_state_t;

void sync_init(void);
void sync_reset(void);                        // zero both tick counters
sync_state_t sync_update(int base_duty);      // call periodically; returns state
sync_state_t sync_get_state(void);
void sync_clear_fault(void);
int32_t sync_get_delta(void);                 // ticks_A - ticks_B
