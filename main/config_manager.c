#include <stdio.h>
#include <string.h>

#include "config_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "CONFIG";
static const char *NVS_NAMESPACE = "storage";

// ------------------------------------------------------------
// Copia RAM privada del módulo.
// NVS solo se toca desde este archivo.
// ------------------------------------------------------------
static system_config_t s_config;

// ------------------------------------------------------------
// Carga los valores por defecto en la copia RAM.
// ------------------------------------------------------------
void config_set_defaults(void)
{
    memset(&s_config, 0, sizeof(s_config));

    s_config.led_blink_time     = LED_BLINK_TIME_DEFAULT;
    s_config.led_blink_quantity = LED_BLINK_QUANTITY_DEFAULT;
    s_config.led_color          = LED_COLOR_DEFAULT;
    s_config.log_level          = 0;

    strncpy(s_config.wifi_ssid, WIFI_SSID_DEFAULT, sizeof(s_config.wifi_ssid) - 1);
    s_config.wifi_ssid[sizeof(s_config.wifi_ssid) - 1] = '\0';

    strncpy(s_config.wifi_pass, WIFI_PASS_DEFAULT, sizeof(s_config.wifi_pass) - 1);
    s_config.wifi_pass[sizeof(s_config.wifi_pass) - 1] = '\0';
}

// ------------------------------------------------------------
// Copia la RAM privada hacia una estructura externa.
// ------------------------------------------------------------
void config_get_copy(system_config_t *config)
{
    if (config == NULL)
    {
        return;
    }

    *config = s_config;
}

// ------------------------------------------------------------
// Copia una estructura externa hacia la RAM privada.
// ------------------------------------------------------------
void config_set_copy(const system_config_t *config)
{
    if (config == NULL)
    {
        return;
    }

    s_config = *config;
}

// ------------------------------------------------------------
// Inicializa NVS y carga configuración.
// ------------------------------------------------------------
esp_err_t config_init(system_config_t *config)
{
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);

    config_set_defaults();

    err = config_load();
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Se usaran valores por defecto");
    }

    if (config != NULL)
    {
        *config = s_config;
    }

    printf("\n");
    printf("**********************************************\n");
    printf("* SISTEMA DE CONTROL DE ENSAYO               *\n");
    printf("* Version: %-28s *\n", SW_VERSION);
    printf("**********************************************\n");
    printf(" WiFi SSID: %s\n", s_config.wifi_ssid);
    printf(" Parametros: %lums | %lu blinks | Color: %lu\n",
           (unsigned long)s_config.led_blink_time,
           (unsigned long)s_config.led_blink_quantity,
           (unsigned long)s_config.led_color);
    printf("**********************************************\n\n");

    return ESP_OK;
}

// ------------------------------------------------------------
// Carga la configuración desde NVS hacia la copia RAM privada.
// Si no encuentra una configuración válida, deja defaults.
// ------------------------------------------------------------
esp_err_t config_load(void)
{
    nvs_handle_t handle;
    esp_err_t err;
    uint32_t magic = 0;
    size_t size;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "No se pudo abrir NVS, usando defaults");
        return err;
    }

    err = nvs_get_u32(handle, "magic", &magic);
    if (err != ESP_OK || magic != MAGIC_NUMBER)
    {
        ESP_LOGW(TAG, "Primer inicio o config invalida. Cargando defaults...");

        nvs_set_u32(handle, "magic", MAGIC_NUMBER);
        nvs_set_u32(handle, "b_time", s_config.led_blink_time);
        nvs_set_u32(handle, "b_qty", s_config.led_blink_quantity);
        nvs_set_u32(handle, "l_color", s_config.led_color);
        nvs_set_u32(handle, "log_lvl", s_config.log_level);
        nvs_set_str(handle, "w_ssid", s_config.wifi_ssid);
        nvs_set_str(handle, "w_pass", s_config.wifi_pass);

        err = nvs_commit(handle);
        nvs_close(handle);
        return err;
    }

    ESP_LOGI(TAG, "Configuracion encontrada en NVS. Cargando...");

    // Si alguna clave falla, mantenemos el valor default ya cargado.
    nvs_get_u32(handle, "b_time", &s_config.led_blink_time);
    nvs_get_u32(handle, "b_qty", &s_config.led_blink_quantity);
    nvs_get_u32(handle, "l_color", &s_config.led_color);
    nvs_get_u32(handle, "log_lvl", &s_config.log_level);

    size = sizeof(s_config.wifi_ssid);
    nvs_get_str(handle, "w_ssid", s_config.wifi_ssid, &size);

    size = sizeof(s_config.wifi_pass);
    nvs_get_str(handle, "w_pass", s_config.wifi_pass, &size);

    nvs_close(handle);
    return ESP_OK;
}

// ------------------------------------------------------------
// Guarda una configuración en NVS.
// Si recibe un puntero externo, primero actualiza la copia RAM.
// ------------------------------------------------------------
esp_err_t config_save(system_config_t *config)
{
    nvs_handle_t handle;
    esp_err_t err;

    if (config != NULL)
    {
        s_config = *config;
    }

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        return err;
    }

    nvs_set_u32(handle, "magic", MAGIC_NUMBER);
    nvs_set_u32(handle, "b_time", s_config.led_blink_time);
    nvs_set_u32(handle, "b_qty", s_config.led_blink_quantity);
    nvs_set_u32(handle, "l_color", s_config.led_color);
    nvs_set_u32(handle, "log_lvl", s_config.log_level);
    nvs_set_str(handle, "w_ssid", s_config.wifi_ssid);
    nvs_set_str(handle, "w_pass", s_config.wifi_pass);

    err = nvs_commit(handle);
    nvs_close(handle);

    return err;
}

// ============================================================
// GETTERS
// ============================================================

uint32_t config_get_led_blink_time(void)
{
    return s_config.led_blink_time;
}

uint32_t config_get_led_blink_quantity(void)
{
    return s_config.led_blink_quantity;
}

uint32_t config_get_led_color(void)
{
    return s_config.led_color;
}

uint32_t config_get_log_level(void)
{
    return s_config.log_level;
}

const char *config_get_wifi_ssid(void)
{
    return s_config.wifi_ssid;
}

const char *config_get_wifi_pass(void)
{
    return s_config.wifi_pass;
}

// ============================================================
// SETTERS
// ============================================================

void config_set_led_blink_time(uint32_t value)
{
    s_config.led_blink_time = value;
}

void config_set_led_blink_quantity(uint32_t value)
{
    s_config.led_blink_quantity = value;
}

void config_set_led_color(uint32_t value)
{
    s_config.led_color = value;
}

void config_set_log_level(uint32_t value)
{
    s_config.log_level = value;
}

void config_set_wifi_ssid(const char *value)
{
    if (value == NULL)
    {
        return;
    }

    strncpy(s_config.wifi_ssid, value, sizeof(s_config.wifi_ssid) - 1);
    s_config.wifi_ssid[sizeof(s_config.wifi_ssid) - 1] = '\0';
}

void config_set_wifi_pass(const char *value)
{
    if (value == NULL)
    {
        return;
    }

    strncpy(s_config.wifi_pass, value, sizeof(s_config.wifi_pass) - 1);
    s_config.wifi_pass[sizeof(s_config.wifi_pass) - 1] = '\0';
}