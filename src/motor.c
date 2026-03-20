#include "motor.h"
#include "hal_esp32.h"
#include <stdlib.h>

#define SOFT_STOP_STEP   5    // duty units reduced per soft stop tick
#define SOFT_STOP_MIN    10   // below this just cut to zero

static int s_duty[2] = {0, 0};

static void apply(int motor)
{
    int d = s_duty[motor];
    if (d > 0) {
        hal_motor_set_pwm(motor, d, 0);
    } else if (d < 0) {
        hal_motor_set_pwm(motor, 0, -d);
    } else {
        hal_motor_set_pwm(motor, 0, 0);
    }
}

void motor_init(void)
{
    hal_motor_enable(MOTOR_A, 1);
    hal_motor_enable(MOTOR_B, 1);
    s_duty[0] = 0;
    s_duty[1] = 0;
}

void motor_set(int motor, int duty)
{
    if (duty >  100) duty =  100;
    if (duty < -100) duty = -100;
    s_duty[motor] = duty;
    apply(motor);
}

void motor_stop_immediate(int motor)
{
    s_duty[motor] = 0;
    apply(motor);
}

// Call this on a timer tick (e.g. every 20ms) while stopping
// Returns 1 when fully stopped, 0 while still ramping
int motor_soft_stop_tick(int motor)
{
    int d = s_duty[motor];
    if (d == 0) return 1;

    if (abs(d) <= SOFT_STOP_MIN) {
        s_duty[motor] = 0;
    } else if (d > 0) {
        s_duty[motor] = d - SOFT_STOP_STEP;
    } else {
        s_duty[motor] = d + SOFT_STOP_STEP;
    }
    apply(motor);
    return (s_duty[motor] == 0) ? 1 : 0;
}

int motor_get_duty(int motor)
{
    return s_duty[motor];
}
