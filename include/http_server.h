#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Commands the HTTP server posts to the desk task
typedef enum {
    CMD_NONE = 0,
    CMD_UP,
    CMD_DOWN,
    CMD_STOP,
    CMD_SAVE_SIT,
    CMD_SAVE_STAND,
    CMD_GOTO_SIT,
    CMD_GOTO_STAND,
} desk_cmd_t;

// Call once after WiFi is connected.
// cmd_queue receives desk_cmd_t values from HTTP handlers.
void http_server_start(QueueHandle_t cmd_queue);
void http_server_stop(void);
