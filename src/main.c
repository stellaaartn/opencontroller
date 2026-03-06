#ifndef UNIT_TEST

#include "motor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


static const char *TAG = "tag";

void app_main(void)
{
    ESP_LOGI(TAG, "main.c app_main()");
    hal_init();
    motor_init();
    sync_init();


}

#endif
