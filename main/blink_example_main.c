#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "app_queue.h"
#include "app_runtime.h"
#include "app_events.h"
#include "config_manager.h"
#include "core_task.h"
#include "web_server.h"
#include "app_network.h"
#include "rainmaker_adapter.h"

static const char *TAG = "MAIN";

/*
al cambiar el cmake

rmdir /s /q build
idf.py reconfigure
idf.py build
*/

/* TEMPORAL:
 * 0 = no borrar nada
 * 1 = borrar SOLO namespace "storage" en este arranque
 */
#define RESET_ONLY_MY_CONFIG_NVS 0

/* Prototipo temporal para no depender del header ahora */
esp_err_t app_config_reset_storage(void);

void app_main(void)
{
    ESP_LOGI("BOOTCHK", "SW_VERSION=%s", SW_VERSION);
    ESP_LOGI("BOOTCHK", "Compiled: %s %s", __DATE__, __TIME__);

    ESP_LOGI(TAG, "Inicializando Fase 5");

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

#if RESET_ONLY_MY_CONFIG_NVS
    ESP_LOGW(TAG, "RESET_ONLY_MY_CONFIG_NVS=1 -> borrando solo storage");
    ESP_ERROR_CHECK(app_config_reset_storage());
#endif

    system_config_t boot_config;
    ESP_ERROR_CHECK(app_config_init(&boot_config));

    app_runtime_init();

    app_network_init();

    if (!app_queue_init())
    {
        ESP_LOGE(TAG, "No se pudo crear la cola interna");
        return;
    }

    if (!app_events_init())
    {
        ESP_LOGE(TAG, "No se pudo registrar el sistema de eventos");
        return;
    }

    core_task_start();

    if (!rainmaker_adapter_init())
    {
        ESP_LOGE(TAG, "Fallo rainmaker_adapter_init()");
        return;
    }

    app_network_start(POP_TYPE_MAC);

    /* Para validacion local del ensayo */
    start_webserver();

    ESP_LOGI(TAG, "Sistema iniciado");
}