#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "core_task.h"
#include "app_cmd.h"
#include "app_queue.h"
#include "app_runtime.h"
#include "app_events.h"

#include "config_manager.h"
#include "led_control.h"
#include "timer_control.h"

static const char *TAG = "CORE_TASK";

static my_led_t s_status_led;
static my_timer_t s_blink_timer;

static void core_apply_config(const app_apply_config_t *cfg)
{
    if (cfg == NULL)
    {
        return;
    }

    config_set_led_blink_time(cfg->led_blink_time);
    config_set_led_blink_quantity(cfg->led_blink_quantity);
    config_set_led_color(cfg->led_color);

    if (cfg->save_to_nvs)
    {
        config_save(NULL);
    }

    if (!app_runtime_is_running())
    {
        led_set_color(&s_status_led, (led_color_t)config_get_led_color());
        led_off(&s_status_led);
    }
}

static void core_process_command(const app_cmd_t *cmd)
{
    if (cmd == NULL)
    {
        return;
    }

    ESP_LOGI(TAG, "CORE processing INTERNAL CMD type=%d", cmd->type);

    switch (cmd->type)
    {
        case APP_CMD_START:

            ESP_LOGI(TAG, "INTERNAL CMD START");

            if (!app_runtime_is_running())
            {
                app_runtime_set_running(true);
                app_runtime_set_remaining(0);
                app_runtime_set_state(APP_STATE_INIT);
                app_runtime_set_domain_status(APP_DOMAIN_RUNNING);
                app_runtime_set_last_result(APP_RESULT_NONE);
            }
            else
            {
                ESP_LOGW(TAG, "START ignorado: ensayo ya en ejecucion");
                app_runtime_set_last_result(APP_RESULT_REJECTED);
            }

            break;

        case APP_CMD_STOP:
        {
            bool was_running = app_runtime_is_running();

            ESP_LOGI(TAG, "INTERNAL CMD STOP");

            app_runtime_set_running(false);
            app_runtime_set_remaining(0);
            app_runtime_set_state(APP_STATE_INIT);
            app_runtime_set_domain_status(APP_DOMAIN_IDLE);

            led_off(&s_status_led);

            if (was_running)
            {
                app_runtime_set_last_result(APP_RESULT_STOPPED);
                app_events_publish_ensayo_stopped();
            }

            break;
        }

        case APP_CMD_APPLY_CONFIG:

            ESP_LOGI(TAG, "INTERNAL CMD APPLY_CONFIG");

            core_apply_config(&cmd->data.apply_config);

            break;

        case APP_CMD_NONE:
        default:
            break;
    }
}

static void core_run_fsm(void)
{
    switch (app_runtime_get_state())
    {
        case APP_STATE_INIT:

            if (app_runtime_is_running())
            {
                app_runtime_set_remaining(config_get_led_blink_quantity());
                app_runtime_set_state(APP_STATE_IDLE);
            }

            break;

        case APP_STATE_IDLE:

            ESP_LOGI(TAG,
                     "ENSAYO START color=%lu pulses=%lu blink=%lums",
                     (unsigned long)config_get_led_color(),
                     (unsigned long)config_get_led_blink_quantity(),
                     (unsigned long)config_get_led_blink_time());

            app_runtime_set_domain_status(APP_DOMAIN_RUNNING);
            app_runtime_set_last_result(APP_RESULT_NONE);

            app_events_publish_ensayo_started();

            led_set_color(&s_status_led, (led_color_t)config_get_led_color());

            timer_start(&s_blink_timer);

            led_on(&s_status_led);

            app_runtime_set_state(APP_STATE_LED_ON);

            break;

        case APP_STATE_LED_ON:

            if (timer_expired(&s_blink_timer, config_get_led_blink_time()))
            {
                led_off(&s_status_led);

                timer_start(&s_blink_timer);

                app_runtime_set_state(APP_STATE_LED_OFF);
            }

            break;

        case APP_STATE_LED_OFF:

            if (timer_expired(&s_blink_timer, config_get_led_blink_time()))
            {
                uint32_t restantes = app_runtime_get_remaining();

                if (restantes > 0)
                {
                    restantes--;

                    app_runtime_set_remaining(restantes);

                    ESP_LOGI(TAG,
                             "PULSE remaining=%lu",
                             (unsigned long)restantes);
                }

                if (restantes > 0)
                {
                    led_on(&s_status_led);

                    timer_start(&s_blink_timer);

                    app_runtime_set_state(APP_STATE_LED_ON);
                }
                else
                {
                    ESP_LOGI(TAG, "ENSAYO FINISHED");

                    app_runtime_set_running(false);
                    app_runtime_set_state(APP_STATE_INIT);
                    app_runtime_set_domain_status(APP_DOMAIN_IDLE);
                    app_runtime_set_last_result(APP_RESULT_FINISHED);

                    led_off(&s_status_led);

                    app_events_publish_ensayo_finished();
                }
            }

            break;

        default:

            app_runtime_set_state(APP_STATE_INIT);

            break;
    }
}

static void core_task(void *pvParameters)
{
    app_cmd_t cmd;

    (void)pvParameters;

    led_init(&s_status_led, 48);
    led_set_color(&s_status_led, (led_color_t)config_get_led_color());
    led_off(&s_status_led);

    while (1)
    {
        while (xQueueReceive(app_queue_get(), &cmd, 0) == pdPASS)
        {
            core_process_command(&cmd);
        }

        core_run_fsm();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void core_task_start(void)
{
    xTaskCreatePinnedToCore(
        core_task,
        "core_task",
        4096,
        NULL,
        5,
        NULL,
        1
    );
}