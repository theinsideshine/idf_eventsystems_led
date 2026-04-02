#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>
#include "esp_err.h"

// ============================================================
// DEFINES DE VALORES POR DEFECTO
// ============================================================
// Subimos el magic porque cambió la estructura persistida
#define MAGIC_NUMBER                564

#define LED_BLINK_TIME_DEFAULT      1010
#define LED_BLINK_QUANTITY_DEFAULT  3
#define LED_COLOR_DEFAULT           1

// Defaults de WiFi
#define WIFI_SSID_DEFAULT           "Pablo"
#define WIFI_PASS_DEFAULT           "01410398716"

// ============================================================
// VERSIÓN DEL SISTEMA
// ============================================================
#define SW_VERSION                  "5.0.10"

// ============================================================
// ESTRUCTURA DE CONFIGURACIÓN PERSISTENTE
// ============================================================
// En Fase 2 dejamos acá solo configuración persistente.
// El estado runtime (running, state, remaining) va en app_runtime.
typedef struct
{
    uint32_t led_blink_time;
    uint32_t led_blink_quantity;
    uint32_t led_color;
    uint32_t log_level;
    char wifi_ssid[32];
    char wifi_pass[64];

} system_config_t;

// ============================================================
// API PRINCIPAL
// ============================================================
esp_err_t config_init(system_config_t *config);
esp_err_t config_save(system_config_t *config);

// ============================================================
// API INTERNA DE MEMORIA / COPIA RAM
// ============================================================
esp_err_t config_load(void);
void config_set_defaults(void);

// Getters
uint32_t config_get_led_blink_time(void);
uint32_t config_get_led_blink_quantity(void);
uint32_t config_get_led_color(void);
uint32_t config_get_log_level(void);
const char *config_get_wifi_ssid(void);
const char *config_get_wifi_pass(void);

// Setters
void config_set_led_blink_time(uint32_t value);
void config_set_led_blink_quantity(uint32_t value);
void config_set_led_color(uint32_t value);
void config_set_log_level(uint32_t value);
void config_set_wifi_ssid(const char *value);
void config_set_wifi_pass(const char *value);

// Copias completas
void config_get_copy(system_config_t *config);
void config_set_copy(const system_config_t *config);

#endif