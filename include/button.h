#pragma once

#include <stdint.h>

typedef enum {
    BTN_UP    = 0,
    BTN_DOWN  = 1,
    BTN_SIT   = 2,
    BTN_STAND = 3,
    BTN_COUNT = 4,
} btn_id_t;

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_TAP,
    BTN_EVENT_HOLD,
    BTN_EVENT_HELD,       // fires every tick while held
    BTN_EVENT_RELEASED,
} btn_event_t;

#define BTN_DEBOUNCE_MS   20
#define BTN_HOLD_MS      800

void        button_init(void);
btn_event_t button_update(btn_id_t btn, uint32_t now_ms); // call every ~10ms
int         button_is_pressed(btn_id_t btn);
