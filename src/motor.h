#pragma once

#include <stdint.h>

// HAL interface — implemented by hal_esp32.c (hardware) or test stub (native)
void hal_motor_init(void);
void hal_motor_set_pwm(int ch_a_duty, int ch_b_duty); // duty 0..100 each channel

// Motor API
void motor_init(void);
void motor_set(int duty); // -100 (full reverse) .. 0 (stop) .. +100 (full forward)
