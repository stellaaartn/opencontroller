#ifndef UNIT_TEST

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "hal_esp32.h"
#include "motor.h"
#include "hall.h"
#include "sync.h"
#include "preset.h"
#include "wifi.h"
#include "http_server.h"

// ---- Config ----------------------------------------------------------------
#define WIFI_SSID       "your_ssid"
#define WIFI_PASSWORD   "your_password"

#define DESK_SPEED      70
#define TASK_PERIOD_MS  10

static const char *TAG = "desk";

// ---- Desk state machine ----------------------------------------------------

typedef enum {
    DESK_IDLE,
    DESK_MOVING_UP,
    DESK_MOVING_DOWN,
    DESK_SOFT_STOPPING,
    DESK_GOTO_PRESET,
    DESK_CORRECTING,
} desk_state_t;

static desk_state_t s_desk      = DESK_IDLE;
static int32_t      s_target    = 0;
static int          s_goto_dir  = 0;

static void start_move(int duty)
{
    sync_reset();
    motor_set(MOTOR_A, duty);
    motor_set(MOTOR_B, duty);
    s_desk = (duty > 0) ? DESK_MOVING_UP : DESK_MOVING_DOWN;
}

static void begin_soft_stop(void)
{
    s_desk = DESK_SOFT_STOPPING;
}

static void goto_preset(preset_id_t id)
{
    int32_t target = preset_load(id);
    if (target == PRESET_NONE) {
        ESP_LOGW(TAG, "No preset saved for id=%d", id);
        return;
    }
    int32_t current = hall_get(MOTOR_A).ticks;
    if (current == target) return;

    s_target   = target;
    s_goto_dir = (target > current) ? +1 : -1;
    start_move(s_goto_dir * DESK_SPEED);
    s_desk = DESK_GOTO_PRESET;
}

// ---- Desk task -------------------------------------------------------------

static void desk_task(void *arg)
{
    QueueHandle_t q = (QueueHandle_t)arg;
    desk_cmd_t cmd;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(TASK_PERIOD_MS));

        // Drain all pending commands from HTTP server
        while (xQueueReceive(q, &cmd, 0) == pdTRUE) {
            switch (cmd) {
            case CMD_UP:
                if (s_desk == DESK_IDLE || s_desk == DESK_SOFT_STOPPING) {
                    start_move(+DESK_SPEED);
                }
                break;
            case CMD_DOWN:
                if (s_desk == DESK_IDLE || s_desk == DESK_SOFT_STOPPING) {
                    start_move(-DESK_SPEED);
                }
                break;
            case CMD_STOP:
                if (s_desk != DESK_IDLE) {
                    begin_soft_stop();
                }
                break;
            case CMD_SAVE_SIT: {
                int32_t pos = hall_get(MOTOR_A).ticks;
                preset_save(PRESET_SIT, pos);
                ESP_LOGI(TAG, "Sit saved: %ld", pos);
                break;
            }
            case CMD_SAVE_STAND: {
                int32_t pos = hall_get(MOTOR_A).ticks;
                preset_save(PRESET_STAND, pos);
                ESP_LOGI(TAG, "Stand saved: %ld", pos);
                break;
            }
            case CMD_GOTO_SIT:
                if (s_desk == DESK_IDLE) goto_preset(PRESET_SIT);
                break;
            case CMD_GOTO_STAND:
                if (s_desk == DESK_IDLE) goto_preset(PRESET_STAND);
                break;
            default:
                break;
            }
        }

        // State machine update
        switch (s_desk) {

        case DESK_MOVING_UP:
        case DESK_MOVING_DOWN: {
            hall_check_stall(MOTOR_A);
            hall_check_stall(MOTOR_B);
            if (hall_get(MOTOR_A).stalled || hall_get(MOTOR_B).stalled) {
                ESP_LOGI(TAG, "Stall detected");
                motor_stop_immediate(MOTOR_A);
                motor_stop_immediate(MOTOR_B);
                s_desk = DESK_IDLE;
                break;
            }
            int duty = (s_desk == DESK_MOVING_UP) ? +DESK_SPEED : -DESK_SPEED;
            sync_state_t ss = sync_update(duty);
            if (ss == SYNC_CORRECTING) s_desk = DESK_CORRECTING;
            break;
        }

        case DESK_CORRECTING: {
            int duty = motor_get_duty(MOTOR_A) != 0
                       ? motor_get_duty(MOTOR_A)
                       : motor_get_duty(MOTOR_B);
            sync_state_t ss = sync_update(duty);
            if (ss == SYNC_OK) {
                ESP_LOGI(TAG, "Sync restored");
                s_desk = DESK_IDLE;
            }
            break;
        }

        case DESK_SOFT_STOPPING: {
            int done_a = motor_soft_stop_tick(MOTOR_A);
            int done_b = motor_soft_stop_tick(MOTOR_B);
            if (done_a && done_b) s_desk = DESK_IDLE;
            break;
        }

        case DESK_GOTO_PRESET: {
            hall_check_stall(MOTOR_A);
            hall_check_stall(MOTOR_B);
            int32_t current = hall_get(MOTOR_A).ticks;
            int arrived = (s_goto_dir > 0)
                          ? (current >= s_target)
                          : (current <= s_target);
            if (arrived || hall_get(MOTOR_A).stalled) {
                begin_soft_stop();
                break;
            }
            sync_update(s_goto_dir * DESK_SPEED);
            break;
        }

        default:
            break;
        }

        // Periodic log
        static int log_ctr = 0;
        if (++log_ctr >= 100) {
            log_ctr = 0;
            ESP_LOGI(TAG, "ticks A=%ld B=%ld state=%d",
                     hall_get(MOTOR_A).ticks,
                     hall_get(MOTOR_B).ticks,
                     s_desk);
        }
    }
}

// ---- app_main --------------------------------------------------------------

void app_main(void)
{
    ESP_LOGI(TAG, "Desk controller starting");

    preset_init();  // NVS must init before wifi (both use nvs_flash)
    hal_init();
    motor_init();
    hall_init();
    sync_init();

    if (!wifi_connect(WIFI_SSID, WIFI_PASSWORD)) {
        ESP_LOGE(TAG, "WiFi failed — halting");
        return;
    }

    char ip[16];
    wifi_get_ip(ip, sizeof(ip));
    ESP_LOGI(TAG, "UI ready at http://%s", ip);

    QueueHandle_t cmd_q = xQueueCreate(8, sizeof(desk_cmd_t));
    http_server_start(cmd_q);

    xTaskCreate(desk_task, "desk", 4096, cmd_q, 5, NULL);
}

#endif // UNIT_TEST
