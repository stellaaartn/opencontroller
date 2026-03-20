#include "preset.h"
#include <string.h>

#ifndef UNIT_TEST
#include "nvs_flash.h"
#include "nvs.h"
#define NVS_NAMESPACE "desk"
#endif

static const char *PRESET_KEYS[2] = { "preset_sit", "preset_stand" };

#ifdef UNIT_TEST
// In-memory stub for native tests
static int32_t s_store[2] = { INT32_MIN, INT32_MIN };

void preset_init(void)
{
    s_store[0] = INT32_MIN;
    s_store[1] = INT32_MIN;
}

void preset_save(preset_id_t id, int32_t ticks)
{
    s_store[id] = ticks;
}

int32_t preset_load(preset_id_t id)
{
    return s_store[id];
}

void preset_clear(preset_id_t id)
{
    s_store[id] = INT32_MIN;
}

#else

void preset_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
}

void preset_save(preset_id_t id, int32_t ticks)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_i32(h, PRESET_KEYS[id], ticks);
    nvs_commit(h);
    nvs_close(h);
}

int32_t preset_load(preset_id_t id)
{
    nvs_handle_t h;
    int32_t val = PRESET_NONE;
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK) return PRESET_NONE;
    nvs_get_i32(h, PRESET_KEYS[id], &val);
    nvs_close(h);
    return val;
}

void preset_clear(preset_id_t id)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_erase_key(h, PRESET_KEYS[id]);
    nvs_commit(h);
    nvs_close(h);
}

#endif // UNIT_TEST
