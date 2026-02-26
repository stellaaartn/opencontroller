#include "motor.h"

void motor_init(void)
{
    hal_motor_init();
}

// duty: -100 (full reverse) .. +100 (full forward), 0 = stop
void motor_set(int duty)
{
    if (duty > 100)  duty = 100;
    if (duty < -100) duty = -100;

    if (duty > 0) {
        hal_motor_set_pwm(duty, 0);
    } else if (duty < 0) {
        hal_motor_set_pwm(0, -duty);
    } else {
        hal_motor_set_pwm(0, 0);
    }
}
