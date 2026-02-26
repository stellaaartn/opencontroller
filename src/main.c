#ifndef UNIT_TEST

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

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

#endif
