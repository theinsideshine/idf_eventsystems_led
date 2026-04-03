#include <stdio.h>
#include <string.h>

#include "config_manager.h"
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
// Helper de log de config actual
// ------------------------------------------------------------
static void log_config(const char *prefix)
{
    ESP_LOGI(TAG,
             "%s time=%lu qty=%lu color=%lu log=%lu ssid='%s' pass_len=%u",
             prefix,
             (unsigned long)s_config.led_blink_time,
             (unsigned long)s_config.led_blink_quantity,
             (unsigned long)s_config.led_color,
             (unsigned long)s_config.log_level,
             s_config.wifi_ssid,
             (unsigned int)strlen(s_config.wifi_pass));
}

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

    log_config("DEFAULTS");
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
    log_config("SET_COPY");
}

// ------------------------------------------------------------
// Inicializa la configuracion del sistema.
// NVS ya debe estar inicializada desde app_main().
// ------------------------------------------------------------
esp_err_t app_config_init(system_config_t *config)
{
    ESP_LOGI(TAG, "app_config_init()");

    config_set_defaults();

    esp_err_t err = app_config_load();
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Se usaran valores por defecto");
    }

    if (config != NULL)
    {
        *config = s_config;
    }

    log_config("INIT_FINAL");

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
// Borra SOLO el namespace propio del modulo ("storage").
// No toca RainMaker ni provisioning.
// ------------------------------------------------------------
esp_err_t app_config_reset_storage(void)
{
    nvs_handle_t handle;
    esp_err_t err;

    ESP_LOGW(TAG, "Borrando namespace NVS '%s'...", NVS_NAMESPACE);

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open fallo en reset: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_erase_all(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_erase_all fallo: %s", esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    nvs_close(handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_commit fallo en reset: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGW(TAG, "Namespace '%s' borrado OK", NVS_NAMESPACE);
    return ESP_OK;
}

// ------------------------------------------------------------
// Carga la configuración desde NVS hacia la copia RAM privada.
// Si no encuentra una configuración válida, deja defaults.
// ------------------------------------------------------------
esp_err_t app_config_load(void)
{
    nvs_handle_t handle;
    esp_err_t err;
    uint32_t magic = 0;
    size_t size;

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "No se pudo abrir NVS, usando defaults: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_get_u32(handle, "magic", &magic);
    ESP_LOGI(TAG, "LOAD magic read err=%s magic=0x%08lx",
             esp_err_to_name(err),
             (unsigned long)magic);

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

        if (err == ESP_OK)
        {
            log_config("LOAD_DEFAULTS_SAVED");
        }

        return err;
    }

    ESP_LOGI(TAG, "Configuracion encontrada en NVS. Cargando...");

    err = nvs_get_u32(handle, "b_time", &s_config.led_blink_time);
    ESP_LOGI(TAG, "LOAD b_time err=%s val=%lu",
             esp_err_to_name(err),
             (unsigned long)s_config.led_blink_time);

    err = nvs_get_u32(handle, "b_qty", &s_config.led_blink_quantity);
    ESP_LOGI(TAG, "LOAD b_qty err=%s val=%lu",
             esp_err_to_name(err),
             (unsigned long)s_config.led_blink_quantity);

    err = nvs_get_u32(handle, "l_color", &s_config.led_color);
    ESP_LOGI(TAG, "LOAD l_color err=%s val=%lu",
             esp_err_to_name(err),
             (unsigned long)s_config.led_color);

    err = nvs_get_u32(handle, "log_lvl", &s_config.log_level);
    ESP_LOGI(TAG, "LOAD log_lvl err=%s val=%lu",
             esp_err_to_name(err),
             (unsigned long)s_config.log_level);

    size = sizeof(s_config.wifi_ssid);
    err = nvs_get_str(handle, "w_ssid", s_config.wifi_ssid, &size);
    ESP_LOGI(TAG, "LOAD w_ssid err=%s val='%s'",
             esp_err_to_name(err),
             s_config.wifi_ssid);

    size = sizeof(s_config.wifi_pass);
    err = nvs_get_str(handle, "w_pass", s_config.wifi_pass, &size);
    ESP_LOGI(TAG, "LOAD w_pass err=%s len=%u",
             esp_err_to_name(err),
             (unsigned int)strlen(s_config.wifi_pass));

    nvs_close(handle);

    log_config("LOAD_FINAL");
    return ESP_OK;
}

// ------------------------------------------------------------
// Guarda una configuración en NVS.
// Si recibe un puntero externo, primero actualiza la copia RAM.
// ------------------------------------------------------------
esp_err_t app_config_save(system_config_t *config)
{
    nvs_handle_t handle;
    esp_err_t err;

    if (config != NULL)
    {
        s_config = *config;
    }

    log_config("SAVE_REQUEST");

    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_open fallo en save: %s", esp_err_to_name(err));
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

    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "SAVE commit OK");
        log_config("SAVE_DONE");
    }
    else
    {
        ESP_LOGE(TAG, "SAVE commit fallo: %s", esp_err_to_name(err));
    }

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