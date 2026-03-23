#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_http_server.h"
#include "esp_log.h"

#include "web_server.h"
#include "app_events.h"
#include "app_runtime.h"
#include "config_manager.h"

static const char *TAG = "WEB_SERVER";

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

static const char *get_domain_status_name(app_domain_status_t status)
{
    switch (status)
    {
        case APP_DOMAIN_RUNNING:
            return "RUNNING";

        case APP_DOMAIN_IDLE:
        default:
            return "IDLE";
    }
}

static const char *get_last_result_name(app_result_t result)
{
    switch (result)
    {
        case APP_RESULT_FINISHED:
            return "TERMINADO";

        case APP_RESULT_STOPPED:
            return "DETENIDO";

        case APP_RESULT_REJECTED:
            return "RECHAZADO";

        case APP_RESULT_NONE:
        default:
            return "NINGUNO";
    }
}

static uint32_t get_query_u32(const char *query, const char *key, uint32_t default_value)
{
    char value[16];

    if ((query == NULL) || (key == NULL))
    {
        return default_value;
    }

    if (httpd_query_key_value(query, key, value, sizeof(value)) == ESP_OK)
    {
        return (uint32_t)strtoul(value, NULL, 10);
    }

    return default_value;
}

static esp_err_t get_handler(httpd_req_t *req)
{
    char out_buf[4600];
    app_runtime_t runtime;
    uint32_t led_color = config_get_led_color();
    const char *nombre_color = get_color_name(led_color);

    app_runtime_get_copy(&runtime);

    snprintf(out_buf, sizeof(out_buf),
        "<html><head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<style>"
        "body{font-family:Arial; background:#f0f2f5; display:flex; justify-content:center; padding:20px;}"
        ".card{background:white; padding:30px; border-radius:12px; box-shadow:0 4px 15px rgba(0,0,0,0.1); width:100%%; max-width:440px; text-align:center;}"
        "h2{color:#1a73e8;}"
        ".status{padding:10px; border-radius:8px; margin-bottom:12px; font-weight:bold; color:white;}"
        ".status-idle{background:#28a745;}"
        ".status-run{background:#e67e22;}"
        ".result{padding:10px; border-radius:8px; margin-bottom:20px; font-weight:bold; background:#eef3f8; color:#333;}"
        "select,input{width:100%%; padding:12px; margin-bottom:15px; border:1px solid #ddd; border-radius:6px; box-sizing:border-box; font-size:1rem;}"
        "input[type='submit']{background:#1a73e8; color:white; border:none; cursor:pointer; font-weight:bold;}"
        ".stop-btn{background:#c0392b !important;}"
        ".footer{margin-top:20px; font-size:0.8rem; color:#888; border-top:1px solid #eee; padding-top:15px;}"
        "label{display:block; text-align:left; margin-bottom:5px; color:#666; font-weight:bold;}"
        "p{margin:8px 0;}"
        "</style>"
        "<script>"
        "function updateStatus(){"
            "fetch('/status')"
                ".then(response => response.json())"
                ".then(data => {"
                    "const statusBox = document.getElementById('status-box');"
                    "const domainBox = document.getElementById('domain-box');"
                    "const resultBox = document.getElementById('result-box');"
                    "const remainingBox = document.getElementById('remaining-box');"
                    "if(data.running){"
                        "statusBox.textContent = 'Estado interno: EJECUTANDO';"
                        "statusBox.className = 'status status-run';"
                    "}else{"
                        "statusBox.textContent = 'Estado interno: IDLE';"
                        "statusBox.className = 'status status-idle';"
                    "}"
                    "domainBox.textContent = 'Estado dominio: ' + data.domain_status_name;"
                    "resultBox.textContent = 'Ultimo resultado: ' + data.last_result_name;"
                    "remainingBox.textContent = 'Restantes: ' + data.remaining;"
                "})"
                ".catch(error => console.log('status error', error));"
        "}"
        "setInterval(updateStatus, 2000);"
        "window.onload = updateStatus;"
        "</script>"
        "</head><body>"
        "<div class='card'>"
        "<h2>Control de Ensayo</h2>"
        "<div id='status-box' class='status %s'>Estado interno: %s</div>"
        "<div id='domain-box' class='result'>Estado dominio: %s</div>"
        "<div id='result-box' class='result'>Ultimo resultado: %s</div>"
        "<p>Color activo: <b>%s</b></p>"
        "<p id='remaining-box'>Restantes: <b>%lu</b></p>"
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
        "<form action='/stop' method='get'>"
        "<input class='stop-btn' type='submit' value='DETENER'>"
        "</form>"
        "<div class='footer'>Firmware v%s</div>"
        "</div></body></html>",
        runtime.running ? "status-run" : "status-idle",
        runtime.running ? "EJECUTANDO" : "IDLE",
        get_domain_status_name(runtime.domain_status),
        get_last_result_name(runtime.last_result),
        nombre_color,
        (unsigned long)runtime.remaining,
        (unsigned long)config_get_led_blink_time(),
        (unsigned long)config_get_led_blink_quantity(),
        (led_color == 1) ? "selected" : "",
        (led_color == 2) ? "selected" : "",
        (led_color == 3) ? "selected" : "",
        (led_color == 4) ? "selected" : "",
        (led_color == 5) ? "selected" : "",
        (led_color == 6) ? "selected" : "",
        (led_color == 7) ? "selected" : "",
        (led_color == 8) ? "selected" : "",
        (led_color == 9) ? "selected" : "",
        (led_color == 10) ? "selected" : "",
        SW_VERSION);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, out_buf, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t status_handler(httpd_req_t *req)
{
    char json[256];
    app_runtime_t runtime;

    app_runtime_get_copy(&runtime);

    snprintf(json, sizeof(json),
             "{\"running\":%s,"
             "\"state\":%d,"
             "\"remaining\":%lu,"
             "\"domain_status\":%d,"
             "\"domain_status_name\":\"%s\","
             "\"last_result\":%d,"
             "\"last_result_name\":\"%s\"}",
             runtime.running ? "true" : "false",
             (int)runtime.state,
             (unsigned long)runtime.remaining,
             (int)runtime.domain_status,
             get_domain_status_name(runtime.domain_status),
             (int)runtime.last_result,
             get_last_result_name(runtime.last_result));

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t save_handler(httpd_req_t *req)
{
    char buf[128];
    app_event_config_updated_t cfg;

    memset(&cfg, 0, sizeof(cfg));

    cfg.led_blink_time = config_get_led_blink_time();
    cfg.led_blink_quantity = config_get_led_blink_quantity();
    cfg.led_color = config_get_led_color();
    cfg.save_to_nvs = true;

    if (httpd_req_get_url_query_str(req, buf, sizeof(buf)) == ESP_OK)
    {
        cfg.led_blink_time =
            get_query_u32(buf, "t", cfg.led_blink_time);

        cfg.led_blink_quantity =
            get_query_u32(buf, "q", cfg.led_blink_quantity);

        cfg.led_color =
            get_query_u32(buf, "c", cfg.led_color);
    }

    ESP_LOGI(TAG,
             "WEB_EVENT CONFIG_UPDATED t=%lu q=%lu color=%lu save=%d",
             (unsigned long)cfg.led_blink_time,
             (unsigned long)cfg.led_blink_quantity,
             (unsigned long)cfg.led_color,
             cfg.save_to_nvs);

    if (!app_events_publish_config_updated(&cfg))
    {
        ESP_LOGE(TAG, "No se pudo publicar CONFIG_UPDATED");
        httpd_resp_send_err(req,
                            HTTPD_500_INTERNAL_SERVER_ERROR,
                            "event publish failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "WEB_EVENT START_REQUESTED");

    if (!app_events_publish_start_requested())
    {
        ESP_LOGE(TAG, "No se pudo publicar START_REQUESTED");
        httpd_resp_send_err(req,
                            HTTPD_500_INTERNAL_SERVER_ERROR,
                            "event publish failed");
        return ESP_FAIL;
    }

    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}

static esp_err_t stop_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WEB_EVENT STOP_REQUESTED");

    if (!app_events_publish_stop_requested())
    {
        ESP_LOGE(TAG, "No se pudo publicar STOP_REQUESTED");
        httpd_resp_send_err(req,
                            HTTPD_500_INTERNAL_SERVER_ERROR,
                            "event publish failed");
        return ESP_FAIL;
    }

    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

void start_webserver(void)
{
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

        httpd_uri_t uri_status = {
            .uri = "/status",
            .method = HTTP_GET,
            .handler = status_handler
        };

        httpd_uri_t uri_save = {
            .uri = "/save",
            .method = HTTP_GET,
            .handler = save_handler
        };

        httpd_uri_t uri_stop = {
            .uri = "/stop",
            .method = HTTP_GET,
            .handler = stop_handler
        };

        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_status);
        httpd_register_uri_handler(server, &uri_save);
        httpd_register_uri_handler(server, &uri_stop);

        ESP_LOGI(TAG, "Servidor web iniciado");
    }
    else
    {
        ESP_LOGE(TAG, "No se pudo iniciar el servidor web");
    }
}