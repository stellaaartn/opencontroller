#ifndef UNIT_TEST  // skip on native test build

#include "motor.h"
#include "driver/mcpwm_prelude.h"
#include "esp_log.h"

#define MOTOR_PWM_GPIO_A  18   // IN1 / forward
#define MOTOR_PWM_GPIO_B  19   // IN2 / reverse
#define PWM_FREQ_HZ       20000
#define PWM_RES_HZ    1000000

static mcpwm_cmpr_handle_t cmp_a, cmp_b;
static uint32_t s_period;

static void hal_init(void)
{
    s_period = PWM_RES_HZ / PWM_FREQ_HZ;
    mcpwm_timer_handle_t timer;
    mcpwm_timer_config_t timer_cfg = {
        .group_id      = 0,
        .clk_src       = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = PWM_RES_HZ,
        .period_ticks  = s_period;
        .count_mode    = MCPWM_TIMER_COUNT_MODE_UP,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_cfg, &timer));

    mcpwm_oper_handle_t oper;
    mcpwm_operator_config_t oper_cfg = { .group_id = 0 };
    ESP_ERROR_CHECK(mcpwm_new_operator(&oper_cfg, &oper));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

    mcpwm_comparator_config_t cmp_cfg = { .flags.update_cmp_on_tez = true };
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmp_cfg, &cmp_a));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &cmp_cfg, &cmp_b));

    mcpwm_gen_handle_t gen_a, gen_b;
    mcpwm_generator_config_t gen_cfg_a = { .gen_gpio_num = MOTOR_PWM_GPIO_A };
    mcpwm_generator_config_t gen_cfg_b = { .gen_gpio_num = MOTOR_PWM_GPIO_B };
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_cfg_a, &gen_a));
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_cfg_b, &gen_b));

    // gen_a: high on timer zero, low on cmp_a match
    mcpwm_gen_timer_event_action_t tea = MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_ZERO, MCPWM_GEN_ACTION_HIGH);
    mcpwm_gen_compare_event_action_t cea = MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmp_a, MCPWM_GEN_ACTION_LOW);
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(gen_a, tea, MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gen_a, cea, MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    mcpwm_gen_timer_event_action_t teb = MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_ZERO, MCPWM_GEN_ACTION_HIGH);
    mcpwm_gen_compare_event_action_t ceb = MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmp_b, MCPWM_GEN_ACTION_LOW);
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(gen_b, teb, MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gen_b, ceb, MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    // start with both outputs at 0
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmp_a, 0));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmp_b, 0));

    ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
}

void hal_set_pwm(int ch_a_duty, int ch_b_duty)
{
    mcpwm_comparator_set_compare_value(cmp_a, s_period * ch_a_duty / 100);
    mcpwm_comparator_set_compare_value(cmp_b, s_period * ch_b_duty / 100);
}


#endif // UNIT_TEST

