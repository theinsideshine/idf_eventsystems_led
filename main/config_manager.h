#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include "esp_err.h"

// --- DEFINES DE VALORES POR DEFECTO ---
#define MAGIC_NUMBER                342
#define LED_BLINK_TIME_DEFAULT      1010
#define LED_BLINK_QUANTITY_DEFAULT  3
#define LED_COLOR_DEFAULT           1
#define ST_TEST_DEFAULT             0
#define ST_MODE_TEST                0
#define ST_MODE_DEMO                200
#define ST_MODE_DEFAULT             ST_MODE_TEST

// Defaults de WiFi
#define WIFI_SSID_DEFAULT           "Pablo"
#define WIFI_PASS_DEFAULT           "01410398716"

// --- VERSIÓN DEL SISTEMA ---
#define SW_VERSION "1.3.0"  // <--- Solo cambiás esto aquí

// --- ESTRUCTURA DE CONFIGURACIÓN ---
typedef struct {
    uint32_t led_blink_time;
    uint32_t led_blink_quantity;
    uint32_t led_color;
    uint32_t st_test;
    uint32_t st_mode;
    uint32_t log_level;
    
    char wifi_ssid[32];
    char wifi_pass[64];
} system_config_t;

// --- PROTOTIPOS DE FUNCIONES ---
esp_err_t config_init(system_config_t *config);
esp_err_t config_save(system_config_t *config);

#endif