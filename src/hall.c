#include "hall.h"
#include "hal_esp32.h"

#ifndef UNIT_TEST
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_attr.h"

static const int HALL_PINS[2][3] = {
    { PIN_HALL_A1, PIN_HALL_A2, PIN_HALL_A3 },
    { PIN_HALL_B1, PIN_HALL_B2, PIN_HALL_B3 },
};
#endif

// Valid forward/reverse state transitions for 3-bit Hall pattern.
// State = (A<<2 | B<<1 | C). 6 valid states out of 8 possible.
// Forward sequence:  1->3->2->6->4->5->1
// Reverse sequence:  1->5->4->6->2->3->1
static const int NEXT_FWD[8] = { 0, 3, 6, 2, 5, 1, 4, 0 };
static const int NEXT_REV[8] = { 0, 5, 3, 1, 6, 4, 2, 0 };

static hall_state_t s_state[2];
static int s_prev_hall[2];

static int read_hall_pins(int motor)
{
#ifndef UNIT_TEST
    int a = gpio_get_level(HALL_PINS[motor][0]);
    int b = gpio_get_level(HALL_PINS[motor][1]);
    int c = gpio_get_level(HALL_PINS[motor][2]);
    return (a << 2) | (b << 1) | c;
#else
    return 0; // overridden in tests
#endif
}

static uint64_t get_time_us(void)
{
#ifndef UNIT_TEST
    return (uint64_t)esp_timer_get_time();
#else
    return 0; // overridden in tests
#endif
}

// Called from ISR or test — determine direction from state transition,
// update counter and timestamp.
void hall_process_tick(int motor, int new_state)
{
    int prev = s_prev_hall[motor];
    s_prev_hall[motor] = new_state;

    if (new_state == 0 || new_state == 7) return; // invalid state

    int dir = 0;
    if (NEXT_FWD[prev] == new_state) dir = +1;
    else if (NEXT_REV[prev] == new_state) dir = -1;
    else return; // glitch, skip

    s_state[motor].ticks    += dir;
    s_state[motor].direction = dir;
    s_state[motor].last_tick_us = get_time_us();
    s_state[motor].stalled   = 0;
}

#ifndef UNIT_TEST
static void IRAM_ATTR hall_isr(void *arg)
{
    int motor = (int)(intptr_t)arg;
    int new_state = read_hall_pins(motor);
    hall_process_tick(motor, new_state);
}

void hall_init(void)
{
    gpio_install_isr_service(0);

    for (int m = 0; m < 2; m++) {
        s_state[m].ticks        = 0;
        s_state[m].direction    = 0;
        s_state[m].last_tick_us = 0;
        s_state[m].stalled      = 0;
        s_prev_hall[m]          = read_hall_pins(m);

        for (int i = 0; i < 3; i++) {
            gpio_config_t cfg = {
                .pin_bit_mask = 1ULL << HALL_PINS[m][i],
                .mode         = GPIO_MODE_INPUT,
                .pull_up_en   = GPIO_PULLUP_ENABLE,
                .intr_type    = GPIO_INTR_ANYEDGE,
            };
            gpio_config(&cfg);
            gpio_isr_handler_add(HALL_PINS[m][i], hall_isr, (void *)(intptr_t)m);
        }
    }
}
#else
void hall_init(void)
{
    for (int m = 0; m < 2; m++) {
        s_state[m].ticks        = 0;
        s_state[m].direction    = 0;
        s_state[m].last_tick_us = 0;
        s_state[m].stalled      = 0;
        s_prev_hall[m]          = 1; // valid start state
    }
}
#endif

void hall_reset(int motor)
{
    s_state[motor].ticks        = 0;
    s_state[motor].direction    = 0;
    s_state[motor].last_tick_us = get_time_us();
    s_state[motor].stalled      = 0;
}

hall_state_t hall_get(int motor)
{
    return s_state[motor];
}

void hall_check_stall(int motor)
{
    uint64_t now     = get_time_us();
    uint64_t elapsed = now - s_state[motor].last_tick_us;
    if (s_state[motor].last_tick_us > 0 && elapsed > STALL_TIMEOUT_US) {
        s_state[motor].stalled = 1;
    }
}
