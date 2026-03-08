#include <string.h>
#include "config_manager.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "CONFIG";
static const char *NVS_NAMESPACE = "storage";

esp_err_t config_init(system_config_t *config) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    nvs_handle_t handle;
    err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    uint32_t magic = 0;
    nvs_get_u32(handle, "magic", &magic);

    if (magic != MAGIC_NUMBER) {
        ESP_LOGW("CONFIG", "Primer inicio. Cargando defaults...");
        
        config->led_blink_time = LED_BLINK_TIME_DEFAULT;
        config->led_blink_quantity = LED_BLINK_QUANTITY_DEFAULT;
        config->led_color = LED_COLOR_DEFAULT;
        config->st_test = ST_TEST_DEFAULT;
        // config->st_mode = ST_MODE_DEFAULT; // Si no lo usas en el .h, podes comentarlo
        
        strncpy(config->wifi_ssid, WIFI_SSID_DEFAULT, sizeof(config->wifi_ssid));
        strncpy(config->wifi_pass, WIFI_PASS_DEFAULT, sizeof(config->wifi_pass));

        nvs_set_u32(handle, "magic", MAGIC_NUMBER);
        nvs_close(handle);
        config_save(config); 
    } else {
        ESP_LOGI("CONFIG", "Configuracion encontrada en NVS. Cargando...");
        nvs_get_u32(handle, "b_time", &config->led_blink_time);
        nvs_get_u32(handle, "b_qty", &config->led_blink_quantity);
        nvs_get_u32(handle, "l_color", &config->led_color);
        nvs_get_u32(handle, "st_test", &config->st_test);
        
        size_t size = sizeof(config->wifi_ssid);
        nvs_get_str(handle, "w_ssid", config->wifi_ssid, &size);
        size = sizeof(config->wifi_pass);
        nvs_get_str(handle, "w_pass", config->wifi_pass, &size);

        nvs_close(handle);
    }

    // --- BLOQUE DE BIENVENIDA POR UART ---
    // Imprime la versión definida en el .h
    printf("\n");
    printf("**********************************************\n");
    printf("* SISTEMA DE CONTROL DE ENSAYO          *\n");
    printf("* Version: %-28s *\n", SW_VERSION);
    printf("**********************************************\n");
    printf(" IP: 192.168.0.143 (WiFi: %s)\n", config->wifi_ssid);
    printf(" Parametros: %lums | %lu blinks | Color: %lu\n", 
            config->led_blink_time, config->led_blink_quantity, config->led_color);
    printf("**********************************************\n\n");

    return ESP_OK;
}

esp_err_t config_save(system_config_t *config) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return err;

    nvs_set_u32(handle, "b_time", config->led_blink_time);
    nvs_set_u32(handle, "b_qty", config->led_blink_quantity);
    nvs_set_u32(handle, "l_color", config->led_color);
    nvs_set_u32(handle, "st_test", config->st_test);
    nvs_set_u32(handle, "st_mode", config->st_mode);
    
    nvs_set_str(handle, "w_ssid", config->wifi_ssid);
    nvs_set_str(handle, "w_pass", config->wifi_pass);

    err = nvs_commit(handle);
    nvs_close(handle);
    return err;
}