#include "esp_log.h"
#include "app_queue.h"
#include "app_runtime.h"
#include "config_manager.h"
#include "core_task.h"
#include "web_server.h"
#include "wifi_manager.h"


/*
rmdir /s /q build
idf.py erase-flash
idf.py build
idf.py flash monitor
*/
static const char *TAG = "MAIN";

void app_main(void)
{
    system_config_t boot_config;

    ESP_LOGI("BOOTCHK", "SW_VERSION=%s", SW_VERSION);
    ESP_LOGI("BOOTCHK", "Compiled: %s %s", __DATE__, __TIME__);

    ESP_LOGI(TAG, "Inicializando Fase 3");

    config_init(&boot_config);
    app_runtime_init();

    if (!app_queue_init())
    {
        ESP_LOGE(TAG, "No se pudo crear la cola principal");
        return;
    }

    wifi_init_sta(&boot_config);
    core_task_start();
    start_webserver();

    ESP_LOGI(TAG, "Sistema iniciado");
}