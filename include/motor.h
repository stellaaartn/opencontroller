#pragma once

#include <stdint.h>

// motor index
#define MOTOR_A  0
#define MOTOR_B  1

// BTS7960: RPWM drives forward, LPWM drives reverse
// duty: -100 (full reverse) .. 0 (stop) .. +100 (full forward)
void motor_init(void);
void motor_set(int motor, int duty);
void motor_stop_immediate(int motor);
void motor_soft_stop(int motor);       // call repeatedly from a task; ramps down
int  motor_get_duty(int motor);
