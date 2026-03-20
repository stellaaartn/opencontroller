#ifndef UNIT_TEST

#include "http_server.h"
#include "hall.h"
#include "sync.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "http";
static httpd_handle_t s_server = NULL;
static QueueHandle_t  s_cmd_q  = NULL;

// ---- Embedded HTML UI ------------------------------------------------------

static const char HTML[] =
"<!DOCTYPE html><html><head>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<title>Desk</title>"
"<style>"
"  body{font-family:sans-serif;display:flex;flex-direction:column;"
"       align-items:center;background:#1a1a2e;color:#eee;margin:0;padding:20px}"
"  h1{margin-bottom:8px;font-size:1.4rem;color:#e94560}"
"  #status{font-size:.85rem;color:#aaa;margin-bottom:24px}"
"  .grid{display:grid;grid-template-columns:1fr 1fr;gap:12px;width:100%;max-width:320px}"
"  button{padding:18px;font-size:1rem;border:none;border-radius:12px;"
"         cursor:pointer;font-weight:bold;transition:opacity .1s}"
"  button:active{opacity:.7}"
"  .up   {background:#0f3460;color:#e94560;grid-column:1/3}"
"  .stop {background:#e94560;color:#fff;grid-column:1/3}"
"  .down {background:#0f3460;color:#e94560;grid-column:1/3}"
"  .sit  {background:#16213e;color:#eee}"
"  .stand{background:#16213e;color:#eee}"
"  .save {background:#0a3d2e;color:#4ecca3;font-size:.8rem}"
"  #pos  {margin-top:20px;font-size:2rem;font-weight:bold;color:#4ecca3}"
"</style></head><body>"
"<h1>Desk Controller</h1>"
"<div id='status'>connecting...</div>"
"<div class='grid'>"
"  <button class='up'    onclick=\"cmd('/up')\">&#9650; UP</button>"
"  <button class='stop'  onclick=\"cmd('/stop')\">&#9632; STOP</button>"
"  <button class='down'  onclick=\"cmd('/down')\">&#9660; DOWN</button>"
"  <button class='sit'   onclick=\"cmd('/goto/sit')\">Sit</button>"
"  <button class='stand' onclick=\"cmd('/goto/stand')\">Stand</button>"
"  <button class='save'  onclick=\"cmd('/save/sit')\">Save Sit</button>"
"  <button class='save'  onclick=\"cmd('/save/stand')\">Save Stand</button>"
"</div>"
"<div id='pos'>--</div>"
"<script>"
"function cmd(path){"
"  fetch(path,{method:'POST'})"
"  .catch(e=>console.error(e))"
"}"
"function poll(){"
"  fetch('/status').then(r=>r.json()).then(d=>{"
"    document.getElementById('pos').textContent=d.ticks+' ticks';"
"    document.getElementById('status').textContent='state: '+d.state;"
"  }).catch(()=>{});"
"}"
"setInterval(poll,500);"
"poll();"
"</script></body></html>";

// ---- Helpers ---------------------------------------------------------------

static esp_err_t send_ok(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr(req, "ok");
    return ESP_OK;
}

static esp_err_t post_cmd(httpd_req_t *req, desk_cmd_t cmd)
{
    xQueueSend(s_cmd_q, &cmd, 0);
    return send_ok(req);
}

// ---- Handlers --------------------------------------------------------------

static esp_err_t handle_root(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, HTML);
    return ESP_OK;
}

static esp_err_t handle_up(httpd_req_t *req)    { return post_cmd(req, CMD_UP); }
static esp_err_t handle_down(httpd_req_t *req)  { return post_cmd(req, CMD_DOWN); }
static esp_err_t handle_stop(httpd_req_t *req)  { return post_cmd(req, CMD_STOP); }

static esp_err_t handle_save_sit(httpd_req_t *req)   { return post_cmd(req, CMD_SAVE_SIT); }
static esp_err_t handle_save_stand(httpd_req_t *req) { return post_cmd(req, CMD_SAVE_STAND); }
static esp_err_t handle_goto_sit(httpd_req_t *req)   { return post_cmd(req, CMD_GOTO_SIT); }
static esp_err_t handle_goto_stand(httpd_req_t *req) { return post_cmd(req, CMD_GOTO_STAND); }

static esp_err_t handle_status(httpd_req_t *req)
{
    char buf[128];
    int32_t ticks = hall_get(MOTOR_A).ticks;
    int state = (int)sync_get_state();
    snprintf(buf, sizeof(buf), "{\"ticks\":%ld,\"state\":%d}", ticks, state);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, buf);
    return ESP_OK;
}

// ---- Route table -----------------------------------------------------------

#define URI(path, method, fn) { \
    .uri = path, .method = method, .handler = fn, .user_ctx = NULL }

static const httpd_uri_t ROUTES[] = {
    URI("/",            HTTP_GET,  handle_root),
    URI("/up",          HTTP_POST, handle_up),
    URI("/down",        HTTP_POST, handle_down),
    URI("/stop",        HTTP_POST, handle_stop),
    URI("/save/sit",    HTTP_POST, handle_save_sit),
    URI("/save/stand",  HTTP_POST, handle_save_stand),
    URI("/goto/sit",    HTTP_POST, handle_goto_sit),
    URI("/goto/stand",  HTTP_POST, handle_goto_stand),
    URI("/status",      HTTP_GET,  handle_status),
};

// ---- Public API ------------------------------------------------------------

void http_server_start(QueueHandle_t cmd_queue)
{
    s_cmd_q = cmd_queue;

    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.max_uri_handlers = 16;

    if (httpd_start(&s_server, &cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return;
    }

    for (int i = 0; i < sizeof(ROUTES)/sizeof(ROUTES[0]); i++) {
        httpd_register_uri_handler(s_server, &ROUTES[i]);
    }

    ESP_LOGI(TAG, "HTTP server started");
}

void http_server_stop(void)
{
    if (s_server) {
        httpd_stop(s_server);
        s_server = NULL;
    }
}

#endif // UNIT_TEST
