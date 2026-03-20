#pragma once

// ---- Motor A (left column) BTS7960 ----------------------------------------
#define PIN_RPWM_A   25
#define PIN_LPWM_A   26
#define PIN_REN_A    27
#define PIN_LEN_A    14

// ---- Motor B (right column) BTS7960 ---------------------------------------
#define PIN_RPWM_B   32
#define PIN_LPWM_B   33
#define PIN_REN_B    18
#define PIN_LEN_B    19

// ---- Hall sensors Motor A --------------------------------------------------
#define PIN_HALL_A1  34
#define PIN_HALL_A2  35
#define PIN_HALL_A3  36

// ---- Hall sensors Motor B --------------------------------------------------
#define PIN_HALL_B1  39
#define PIN_HALL_B2   4
#define PIN_HALL_B3   5

// ---- Buttons ---------------------------------------------------------------
#define PIN_BTN_UP      13
#define PIN_BTN_DOWN    12
#define PIN_BTN_SIT     15
#define PIN_BTN_STAND    2

// ---- PWM config ------------------------------------------------------------
#define PWM_FREQ_HZ      20000
#define PWM_RESOLUTION   MCPWM_TIMER_COUNT_MODE_UP
#define PWM_PERIOD_TICKS 1000   // 1MHz / 20kHz = 50 ticks, scaled to 1000 for 0.1% resolution

void hal_init(void);

// HAL interface for motor (implemented here, stubbed in tests)
void hal_motor_set_pwm(int motor, int rpwm_duty, int lpwm_duty); // motor 0=A 1=B, duty 0..100
void hal_motor_enable(int motor, int enabled);
