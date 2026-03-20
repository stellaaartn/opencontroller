#include "sync.h"
#include "hall.h"
#include "motor.h"
#include <stdlib.h>

static sync_state_t s_state = SYNC_OK;

void sync_init(void)  { s_state = SYNC_OK; }
void sync_clear_fault(void) { s_state = SYNC_OK; }

void sync_reset(void)
{
    hall_reset(MOTOR_A);
    hall_reset(MOTOR_B);
    s_state = SYNC_OK;
}

int32_t sync_get_delta(void)
{
    return hall_get(MOTOR_A).ticks - hall_get(MOTOR_B).ticks;
}

sync_state_t sync_get_state(void) { return s_state; }

// Call this every ~20ms while motors are running.
// base_duty is the commanded duty for both motors.
sync_state_t sync_update(int base_duty)
{
    if (s_state == SYNC_FAULT) return SYNC_FAULT;

    int32_t delta = sync_get_delta(); // positive = A ahead, negative = B ahead
    int32_t abs_delta = delta < 0 ? -delta : delta;

    if (abs_delta > SYNC_TILT_THRESHOLD) {
        // Stop both immediately
        motor_stop_immediate(MOTOR_A);
        motor_stop_immediate(MOTOR_B);
        s_state = SYNC_CORRECTING;
    }

    if (s_state == SYNC_CORRECTING) {
        // Drive lagging motor at low duty until delta clears
        if (abs_delta <= 1) {
            // Back in sync — resume full duty on both
            motor_set(MOTOR_A, base_duty);
            motor_set(MOTOR_B, base_duty);
            s_state = SYNC_OK;
        } else if (delta > 0) {
            // A is ahead — nudge B forward
            int dir = base_duty >= 0 ? 1 : -1;
            motor_set(MOTOR_B, dir * 40);
            motor_stop_immediate(MOTOR_A);
        } else {
            // B is ahead — nudge A forward
            int dir = base_duty >= 0 ? 1 : -1;
            motor_set(MOTOR_A, dir * 40);
            motor_stop_immediate(MOTOR_B);
        }
        return SYNC_CORRECTING;
    }

    // In sync — throttle faster motor if starting to drift
    if (abs_delta > 1) {
        int duty_a = base_duty;
        int duty_b = base_duty;
        if (delta > 0) {
            // A slightly ahead — reduce A
            duty_a = base_duty - SYNC_THROTTLE_STEP;
        } else {
            // B slightly ahead — reduce B
            duty_b = base_duty - SYNC_THROTTLE_STEP;
        }
        motor_set(MOTOR_A, duty_a);
        motor_set(MOTOR_B, duty_b);
    } else {
        motor_set(MOTOR_A, base_duty);
        motor_set(MOTOR_B, base_duty);
    }

    return SYNC_OK;
}
