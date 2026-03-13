#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "config_manager.h"
#include "app_runtime.h"
#include "led_control.h"
#include "timer_control.h"
#include "wifi_manager.h"
#include "web_server.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    system_config_t mi_config;
    my_led_t status_led;
    my_timer_t blink_timer;

    config_init(&mi_config);
    app_runtime_init();

    wifi_init_sta(&mi_config);
    led_init(&status_led, 48);
    start_webserver(&mi_config);

    while (1)
    {
        switch (app_runtime_get_state())
        {
            case APP_STATE_INIT:
                if (app_runtime_is_running())
                {
                    app_runtime_set_remaining(mi_config.led_blink_quantity);
                    app_runtime_set_state(APP_STATE_IDLE);
                }
                break;

            case APP_STATE_IDLE:
                ESP_LOGI(TAG, "--- INICIO ENSAYO ---");

                led_set_color(&status_led, (uint8_t)mi_config.led_color);
                timer_start(&blink_timer);
                led_on(&status_led);

                app_runtime_set_state(APP_STATE_LED_ON);
                break;

            case APP_STATE_LED_ON:
                if (timer_expired(&blink_timer, mi_config.led_blink_time))
                {
                    led_off(&status_led);
                    timer_start(&blink_timer);
                    app_runtime_set_state(APP_STATE_LED_OFF);
                }
                break;

            case APP_STATE_LED_OFF:
                if (timer_expired(&blink_timer, mi_config.led_blink_time))
                {
                    uint32_t restantes = app_runtime_get_remaining();

                    if (restantes > 0)
                    {
                        restantes--;
                        app_runtime_set_remaining(restantes);
                    }

                    if (restantes > 0)
                    {
                        led_on(&status_led);
                        timer_start(&blink_timer);
                        app_runtime_set_state(APP_STATE_LED_ON);
                    }
                    else
                    {
                        ESP_LOGI(TAG, "--- ENSAYO TERMINADO ---");
                        app_runtime_set_running(false);
                        app_runtime_set_state(APP_STATE_INIT);
                    }
                }
                break;

            default:
                app_runtime_set_state(APP_STATE_INIT);
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}