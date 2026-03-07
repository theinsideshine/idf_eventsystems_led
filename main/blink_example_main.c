#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "config_manager.h"
#include "led_control.h"
#include "timer_control.h"
#include "wifi_manager.h" // <-- Nuevo

static const char *TAG = "MAIN";

void app_main(void) {
    system_config_t mi_config;
    my_led_t status_led; 
    my_timer_t blink_timer;
    bool led_state = false;

    if (config_init(&mi_config) == ESP_OK) {
        ESP_LOGI(TAG, "Configuracion NVS cargada.");
    }

    // Iniciamos WiFi con los datos de la NVS
    wifi_init_sta(&mi_config); 

    led_init(&status_led, 48);          
    led_set_color(&status_led, (led_color_t)mi_config.led_color); 
    timer_start(&blink_timer);          

    while (1) {
        if (timer_expired(&blink_timer, mi_config.led_blink_time)) { 
            timer_start(&blink_timer); 
            led_state = !led_state;
            led_state ? led_on(&status_led) : led_off(&status_led);
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}