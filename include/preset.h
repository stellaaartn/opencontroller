#pragma once

#include <stdint.h>

typedef enum {
    PRESET_SIT   = 0,
    PRESET_STAND = 1,
} preset_id_t;

#define PRESET_NONE  INT32_MIN   // sentinel: no preset saved

void    preset_init(void);                          // init NVS
void    preset_save(preset_id_t id, int32_t ticks); // save current position
int32_t preset_load(preset_id_t id);                // returns PRESET_NONE if not set
void    preset_clear(preset_id_t id);
