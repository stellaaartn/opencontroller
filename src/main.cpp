#ifndef UNIT_TEST  // skip on native test build

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/mcpwm_prelude.h"
#include "esp_log.h"

#define MOTOR_PWM_GPIO_A  18   // IN1 / forward
#define MOTOR_PWM_GPIO_B  19   // IN2 / reverse
#define PWM_FREQ_HZ       20000

static const char *TAG = "motor";

// --- motor driver -----------------------------------------------------------

static mcpwm_cmpr_handle_t cmp_a, cmp_b;

static void motor_init(void)
{
    mcpwm_timer_handle_t timer;
    mcpwm_timer_config_t timer_cfg = {
        .group_id      = 0,
        .clk_src       = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,   // 1 MHz tick
        .period_ticks  = 1000000 / PWM_FREQ_HZ,
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

// duty: -100 (full reverse) .. +100 (full forward), 0 = stop
void motor_set(int duty)
{
    uint32_t period = 1000000 / PWM_FREQ_HZ;
    if (duty > 0) {
        uint32_t ticks = (uint32_t)(period * duty / 100);
        mcpwm_comparator_set_compare_value(cmp_a, ticks);
        mcpwm_comparator_set_compare_value(cmp_b, 0);
    } else if (duty < 0) {
        uint32_t ticks = (uint32_t)(period * (-duty) / 100);
        mcpwm_comparator_set_compare_value(cmp_a, 0);
        mcpwm_comparator_set_compare_value(cmp_b, ticks);
    } else {
        mcpwm_comparator_set_compare_value(cmp_a, 0);
        mcpwm_comparator_set_compare_value(cmp_b, 0);
    }
}

// ---------------------------------------------------------------------------

void app_main(void)
{
    motor_init();

    while (1) {
        ESP_LOGI(TAG, "UP  50%%");
        motor_set(50);
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_LOGI(TAG, "STOP");
        motor_set(0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "DOWN 50%%");
        motor_set(-50);
        vTaskDelay(pdMS_TO_TICKS(2000));

        ESP_LOGI(TAG, "STOP");
        motor_set(0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#endif // UNIT_TEST

