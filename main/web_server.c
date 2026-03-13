#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "web_server.h"
#include "config_manager.h"
#include "app_runtime.h"

static const char *TAG = "WEB_SERVER";
static system_config_t *server_config;

static const char *get_color_name(uint32_t color)
{
    switch (color)
    {
        case 1:  return "ROJO";
        case 2:  return "AMARILLO";
        case 3:  return "VERDE";
        case 4:  return "BLANCO";
        case 5:  return "AZUL";
        case 6:  return "CIAN";
        case 7:  return "MAGENTA";
        case 8:  return "NARANJA";
        case 9:  return "VIOLETA";
        case 10: return "GRIS";
        default: return "OFF";
    }
}

esp_err_t get_handler(httpd_req_t *req)
{
    char out_buf[4096];
    bool running = app_runtime_is_running();
    const char *nombre_color = get_color_name(server_config->led_color);

    snprintf(out_buf, sizeof(out_buf),
        "<html><head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>"
        "body{font-family:Arial; background:#f0f2f5; display:flex; justify-content:center; padding:20px;}"
        ".card{background:white; padding:30px; border-radius:12px; box-shadow:0 4px 15px rgba(0,0,0,0.1); width:100%%; max-width:400px; text-align:center;}"
        "h2{color:#1a73e8;}"
        ".status{padding:10px; border-radius:8px; margin-bottom:20px; font-weight:bold; color:white;}"
        ".status-idle{background:#28a745;}"
        ".status-run{background:#e67e22;}"
        "select, input{width:100%%; padding:12px; margin-bottom:15px; border:1px solid #ddd; border-radius:6px; box-sizing:border-box; font-size:1rem;}"
        "input[type='submit']{background:#1a73e8; color:white; border:none; cursor:pointer; font-weight:bold;}"
        ".footer{margin-top:20px; font-size:0.8rem; color:#888; border-top:1px solid #eee; padding-top:15px;}"
        "label{display:block; text-align:left; margin-bottom:5px; color:#666; font-weight:bold;}"
        "</style>"
        "<script>"
        "function updateStatus(){"
            "fetch('/status')"
                ".then(response => response.json())"
                ".then(data => {"
                    "const statusBox = document.getElementById('status-box');"
                    "if(data.running){"
                        "statusBox.textContent = 'Estado: EJECUTANDO';"
                        "statusBox.className = 'status status-run';"
                    "}else{"
                        "statusBox.textContent = 'Estado: IDLE';"
                        "statusBox.className = 'status status-idle';"
                    "}"
                "})"
                ".catch(error => console.log('status error', error));"
        "}"
        "setInterval(updateStatus, 1000);"
        "window.onload = updateStatus;"
        "</script>"
        "</head><body>"
        "<div class='card'>"
        "<h2>Control de Ensayo</h2>"
        "<div id='status-box' class='status %s'>Estado: %s</div>"
        "<p>Color activo: <b>%s</b></p>"
        "<form action='/save' method='get'>"
        "<label>Tiempo Blink (ms):</label><input type='number' name='t' value='%lu'>"
        "<label>Cantidad de Pulso:</label><input type='number' name='q' value='%lu'>"
        "<label>Seleccionar Color:</label>"
        "<select name='c'>"
        "<option value='1' %s>ROJO</option>"
        "<option value='2' %s>AMARILLO</option>"
        "<option value='3' %s>VERDE</option>"
        "<option value='4' %s>BLANCO</option>"
        "<option value='5' %s>AZUL</option>"
        "<option value='6' %s>CIAN</option>"
        "<option value='7' %s>MAGENTA</option>"
        "<option value='8' %s>NARANJA</option>"
        "<option value='9' %s>VIOLETA</option>"
        "<option value='10' %s>GRIS</option>"
        "</select>"
        "<input type='submit' value='GUARDAR E INICIAR'>"
        "</form>"
        "<div class='footer'>Firmware v%s</div>"
        "</div></body></html>",
        running ? "status-run" : "status-idle",
        running ? "EJECUTANDO" : "IDLE",
        nombre_color,
        (unsigned long)server_config->led_blink_time,
        (unsigned long)server_config->led_blink_quantity,
        (server_config->led_color == 1) ? "selected" : "",
        (server_config->led_color == 2) ? "selected" : "",
        (server_config->led_color == 3) ? "selected" : "",
        (server_config->led_color == 4) ? "selected" : "",
        (server_config->led_color == 5) ? "selected" : "",
        (server_config->led_color == 6) ? "selected" : "",
        (server_config->led_color == 7) ? "selected" : "",
        (server_config->led_color == 8) ? "selected" : "",
        (server_config->led_color == 9) ? "selected" : "",
        (server_config->led_color == 10) ? "selected" : "",
        SW_VERSION);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, out_buf, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t status_handler(httpd_req_t *req)
{
    char json[128];
    app_runtime_t runtime;

    app_runtime_get_copy(&runtime);

    snprintf(json, sizeof(json),
             "{\"running\":%s,\"state\":%d,\"remaining\":%lu}",
             runtime.running ? "true" : "false",
             (int)runtime.state,
             (unsigned long)runtime.remaining);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t save_handler(httpd_req_t *req)
{
    char buf[128];

    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK)
    {
        char val[16];

        if (httpd_query_key_value(buf, "t", val, sizeof(val)) == ESP_OK)
            server_config->led_blink_time = strtoul(val, NULL, 10);

        if (httpd_query_key_value(buf, "q", val, sizeof(val)) == ESP_OK)
            server_config->led_blink_quantity = strtoul(val, NULL, 10);

        if (httpd_query_key_value(buf, "c", val, sizeof(val)) == ESP_OK)
            server_config->led_color = strtoul(val, NULL, 10);

        config_save(server_config);

        app_runtime_set_running(true);
        app_runtime_set_state(APP_STATE_INIT);
    }

    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

void start_webserver(system_config_t *config)
{
    server_config = config;

    httpd_handle_t server = NULL;
    httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();

    http_config.stack_size = 10240;

    if (httpd_start(&server, &http_config) == ESP_OK)
    {
        httpd_uri_t uri_get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = get_handler
        };
        httpd_register_uri_handler(server, &uri_get);

        httpd_uri_t uri_status = {
            .uri = "/status",
            .method = HTTP_GET,
            .handler = status_handler
        };
        httpd_register_uri_handler(server, &uri_status);

        httpd_uri_t uri_save = {
            .uri = "/save",
            .method = HTTP_GET,
            .handler = save_handler
        };
        httpd_register_uri_handler(server, &uri_save);

        ESP_LOGI(TAG, "Servidor web iniciado");
    }
    else
    {
        ESP_LOGE(TAG, "No se pudo iniciar el servidor web");
    }
}
