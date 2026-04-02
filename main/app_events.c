#include <string.h>

#include "freertos/FreeRTOS.h"

#include "esp_event.h"
#include "esp_log.h"

#include "app_events.h"
#include "app_cmd.h"
#include "app_queue.h"
#include "app_runtime.h"

#include "rainmaker_adapter.h"

ESP_EVENT_DEFINE_BASE(APP_EVENTS);

static const char *TAG = "APP_EVENTS";

static void app_internal_event_handler(void *handler_arg,
                                       esp_event_base_t event_base,
                                       int32_t event_id,
                                       void *event_data)
{
    (void)handler_arg;

    if (event_base != APP_EVENTS)
    {
        return;
    }

    switch ((app_event_id_t)event_id)
    {
        case APP_EVENT_CONFIG_UPDATED:
        {
            const app_event_config_updated_t *cfg =
                (const app_event_config_updated_t *)event_data;

            if (cfg == NULL)
            {
                ESP_LOGW(TAG, "CONFIG_UPDATED sin datos");
                return;
            }

            app_cmd_t cmd;
            memset(&cmd, 0, sizeof(cmd));

            cmd.type = APP_CMD_APPLY_CONFIG;
            cmd.data.apply_config.led_blink_time = cfg->led_blink_time;
            cmd.data.apply_config.led_blink_quantity = cfg->led_blink_quantity;
            cmd.data.apply_config.led_color = cfg->led_color;
            cmd.data.apply_config.save_to_nvs = cfg->save_to_nvs;

            ESP_LOGI(TAG,
                     "EVENT CONFIG_UPDATED -> CMD APPLY_CONFIG t=%lu q=%lu c=%lu save=%d",
                     (unsigned long)cfg->led_blink_time,
                     (unsigned long)cfg->led_blink_quantity,
                     (unsigned long)cfg->led_color,
                     cfg->save_to_nvs);

            if (!app_queue_send(&cmd, pdMS_TO_TICKS(100)))
            {
                ESP_LOGE(TAG, "No se pudo enviar APPLY_CONFIG a la cola interna");
            }

            break;
        }

        case APP_EVENT_START_REQUESTED:
        {
            app_cmd_t cmd;
            memset(&cmd, 0, sizeof(cmd));
            cmd.type = APP_CMD_START;

            if (app_runtime_is_running())
            {
                ESP_LOGW(TAG, "START_REQUESTED ignorado: ensayo ya en ejecucion");
                app_runtime_set_last_result(APP_RESULT_REJECTED);
                return;
            }

            app_runtime_set_last_result(APP_RESULT_NONE);

            ESP_LOGI(TAG, "EVENT START_REQUESTED -> CMD START");

            if (!app_queue_send(&cmd, pdMS_TO_TICKS(100)))
            {
                ESP_LOGE(TAG, "No se pudo enviar START a la cola interna");
            }

            break;
        }

        case APP_EVENT_STOP_REQUESTED:
        {
            app_cmd_t cmd;
            memset(&cmd, 0, sizeof(cmd));
            cmd.type = APP_CMD_STOP;

            ESP_LOGI(TAG, "EVENT STOP_REQUESTED -> CMD STOP");

            if (!app_queue_send(&cmd, pdMS_TO_TICKS(100)))
            {
                ESP_LOGE(TAG, "No se pudo enviar STOP a la cola interna");
            }

            break;
        }

        case APP_EVENT_ENSAYO_STARTED:
            ESP_LOGI(TAG, "EVENT ENSAYO_STARTED");
            rainmaker_adapter_report_runtime();
            break;

        case APP_EVENT_ENSAYO_FINISHED:
            ESP_LOGI(TAG, "EVENT ENSAYO_FINISHED");
            rainmaker_adapter_report_runtime();
            break;

        case APP_EVENT_ENSAYO_STOPPED:
            ESP_LOGI(TAG, "EVENT ENSAYO_STOPPED");
            rainmaker_adapter_report_runtime();
            break;

        default:
            ESP_LOGW(TAG, "Evento desconocido id=%ld", (long)event_id);
            break;
    }
}

bool app_events_init(void)
{
    esp_err_t err;

    err = esp_event_handler_register(APP_EVENTS,
                                     APP_EVENT_CONFIG_UPDATED,
                                     app_internal_event_handler,
                                     NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo registrar APP_EVENT_CONFIG_UPDATED");
        return false;
    }

    err = esp_event_handler_register(APP_EVENTS,
                                     APP_EVENT_START_REQUESTED,
                                     app_internal_event_handler,
                                     NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo registrar APP_EVENT_START_REQUESTED");
        return false;
    }

    err = esp_event_handler_register(APP_EVENTS,
                                     APP_EVENT_STOP_REQUESTED,
                                     app_internal_event_handler,
                                     NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo registrar APP_EVENT_STOP_REQUESTED");
        return false;
    }

    err = esp_event_handler_register(APP_EVENTS,
                                     APP_EVENT_ENSAYO_STARTED,
                                     app_internal_event_handler,
                                     NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo registrar APP_EVENT_ENSAYO_STARTED");
        return false;
    }

    err = esp_event_handler_register(APP_EVENTS,
                                     APP_EVENT_ENSAYO_FINISHED,
                                     app_internal_event_handler,
                                     NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo registrar APP_EVENT_ENSAYO_FINISHED");
        return false;
    }

    err = esp_event_handler_register(APP_EVENTS,
                                     APP_EVENT_ENSAYO_STOPPED,
                                     app_internal_event_handler,
                                     NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo registrar APP_EVENT_ENSAYO_STOPPED");
        return false;
    }

    ESP_LOGI(TAG, "Sistema de eventos registrado");
    return true;
}

bool app_events_publish_config_updated(const app_event_config_updated_t *cfg)
{
    if (cfg == NULL)
    {
        return false;
    }

    esp_err_t err = esp_event_post(APP_EVENTS,
                                   APP_EVENT_CONFIG_UPDATED,
                                   (void *)cfg,
                                   sizeof(*cfg),
                                   pdMS_TO_TICKS(100));

    return (err == ESP_OK);
}

bool app_events_publish_start_requested(void)
{
    esp_err_t err = esp_event_post(APP_EVENTS,
                                   APP_EVENT_START_REQUESTED,
                                   NULL,
                                   0,
                                   pdMS_TO_TICKS(100));

    return (err == ESP_OK);
}

bool app_events_publish_stop_requested(void)
{
    esp_err_t err = esp_event_post(APP_EVENTS,
                                   APP_EVENT_STOP_REQUESTED,
                                   NULL,
                                   0,
                                   pdMS_TO_TICKS(100));

    return (err == ESP_OK);
}

bool app_events_publish_ensayo_started(void)
{
    esp_err_t err = esp_event_post(APP_EVENTS,
                                   APP_EVENT_ENSAYO_STARTED,
                                   NULL,
                                   0,
                                   pdMS_TO_TICKS(100));

    return (err == ESP_OK);
}

bool app_events_publish_ensayo_finished(void)
{
    esp_err_t err = esp_event_post(APP_EVENTS,
                                   APP_EVENT_ENSAYO_FINISHED,
                                   NULL,
                                   0,
                                   pdMS_TO_TICKS(100));

    return (err == ESP_OK);
}

bool app_events_publish_ensayo_stopped(void)
{
    esp_err_t err = esp_event_post(APP_EVENTS,
                                   APP_EVENT_ENSAYO_STOPPED,
                                   NULL,
                                   0,
                                   pdMS_TO_TICKS(100));

    return (err == ESP_OK);
}