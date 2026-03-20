#ifndef UNIT_TEST

#include "hal_esp32.h"
#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "hal";

// Two operators, one per motor. Each operator has two comparators (RPWM, LPWM).
static mcpwm_cmpr_handle_t s_cmp_rpwm[2];
static mcpwm_cmpr_handle_t s_cmp_lpwm[2];
static uint32_t s_period;

static void setup_motor_mcpwm(int motor,
                                int rpwm_gpio, int lpwm_gpio,
                                mcpwm_timer_handle_t timer)
{
    mcpwm_oper_handle_t oper;
    mcpwm_operator_config_t oper_cfg = { .group_id = 0 };
    ESP_ERROR_CHECK(mcpwm_new_operator(&oper_cfg, &oper));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    mcpwm_comparator_config_t cmp_cfg = { .flags.update_cmp_on_tez = true };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmp_cfg, &s_cmp_rpwm[motor]));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmp_cfg, &s_cmp_lpwm[motor]));

    mcpwm_gen_handle_t gen_r, gen_l;
    mcpwm_generator_config_t gcfg_r = { .gen_gpio_num = rpwm_gpio };
    mcpwm_generator_config_t gcfg_l = { .gen_gpio_num = lpwm_gpio };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gcfg_r, &gen_r));
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gcfg_l, &gen_l));

    // RPWM: high at zero, low at comparator match
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(gen_r,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_ZERO, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gen_r,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, s_cmp_rpwm[motor], MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    // LPWM: same pattern on its comparator
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(gen_l,
        MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_ZERO, MCPWM_GEN_ACTION_HIGH),
        MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gen_l,
        MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, s_cmp_lpwm[motor], MCPWM_GEN_ACTION_LOW),
        MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    mcpwm_comparator_set_compare_value(s_cmp_rpwm[motor], 0);
    mcpwm_comparator_set_compare_value(s_cmp_lpwm[motor], 0);
}

static void setup_enable_pins(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << PIN_REN_A) | (1ULL << PIN_LEN_A) |
                        (1ULL << PIN_REN_B) | (1ULL << PIN_LEN_B),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);
    // start disabled
    gpio_set_level(PIN_REN_A, 0);
    gpio_set_level(PIN_LEN_A, 0);
    gpio_set_level(PIN_REN_B, 0);
    gpio_set_level(PIN_LEN_B, 0);
}

void hal_init(void)
{
    s_period = 1000000 / PWM_FREQ_HZ;

    mcpwm_timer_handle_t timer;
    mcpwm_timer_config_t timer_cfg = {
        .group_id      = 0,
        .clk_src       = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,
        .period_ticks  = s_period,
        .count_mode    = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_cfg, &timer));

    setup_motor_mcpwm(0, PIN_RPWM_A, PIN_LPWM_A, timer);
    setup_motor_mcpwm(1, PIN_RPWM_B, PIN_LPWM_B, timer);
    setup_enable_pins();

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

    ESP_LOGI(TAG, "HAL init done");
}

void hal_motor_set_pwm(int motor, int rpwm_duty, int lpwm_duty)
{
    uint32_t r = (uint32_t)(s_period * rpwm_duty / 100);
    uint32_t l = (uint32_t)(s_period * lpwm_duty / 100);
    mcpwm_comparator_set_compare_value(s_cmp_rpwm[motor], r);
    mcpwm_comparator_set_compare_value(s_cmp_lpwm[motor], l);
}

void hal_motor_enable(int motor, int enabled)
{
    if (motor == 0) {
        gpio_set_level(PIN_REN_A, enabled);
        gpio_set_level(PIN_LEN_A, enabled);
    } else {
        gpio_set_level(PIN_REN_B, enabled);
        gpio_set_level(PIN_LEN_B, enabled);
    }
}

#endif // UNIT_TEST
