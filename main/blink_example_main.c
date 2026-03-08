#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "config_manager.h"
#include "led_control.h"
#include "timer_control.h"
#include "wifi_manager.h"
#include "web_server.h"

static const char *TAG = "MAIN";

#define ST_INIT     0
#define ST_IDLE     1
#define ST_LED_ON   2
#define ST_LED_OFF  3

void app_main(void) {
    system_config_t mi_config;
    my_led_t status_led; 
    my_timer_t blink_timer;
    uint8_t st_loop = ST_INIT;
    uint32_t blinks_restantes = 0;

    config_init(&mi_config); 
    wifi_init_sta(&mi_config);
    led_init(&status_led, 48); 
    start_webserver(&mi_config);

    while (1) {
        switch (st_loop) {
            case ST_INIT:
                if (mi_config.st_test == 1) {
                    blinks_restantes = mi_config.led_blink_quantity;
                    st_loop = ST_IDLE;
                }
                break;

            case ST_IDLE:
                ESP_LOGI(TAG, "--- INICIO ENSAYO ---");
                // Corregido: usamos led_color como en tu .h
                led_set_color(&status_led, (uint8_t)mi_config.led_color); 
                
                timer_start(&blink_timer);
                led_on(&status_led);
                st_loop = ST_LED_ON;
                break;

            case ST_LED_ON:
                if (timer_expired(&blink_timer, mi_config.led_blink_time)) {
                    led_off(&status_led);
                    timer_start(&blink_timer);
                    st_loop = ST_LED_OFF;
                }
                break;

            case ST_LED_OFF:
                if (timer_expired(&blink_timer, mi_config.led_blink_time)) {
                    blinks_restantes--;
                    if (blinks_restantes > 0) {
                        led_on(&status_led);
                        timer_start(&blink_timer);
                        st_loop = ST_LED_ON;
                    } else {
                        ESP_LOGI(TAG, "--- ENSAYO TERMINADO ---");
                        mi_config.st_test = 0;    
                        config_save(&mi_config); 
                        st_loop = ST_INIT;
                    }
                }
                break;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}