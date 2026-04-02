#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

/* RainMaker */
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_params.h"

#include "app_events.h"
#include "app_runtime.h"
#include "rainmaker_adapter.h"

static const char *TAG = "RAINMAKER";

static esp_rmaker_device_t *s_device = NULL;
static esp_rmaker_param_t *s_power_param = NULL;

static esp_err_t write_cb(const esp_rmaker_device_t *device,
                          const esp_rmaker_param_t *param,
                          const esp_rmaker_param_val_t val,
                          void *priv_data,
                          esp_rmaker_write_ctx_t *ctx)
{
    (void)device;
    (void)priv_data;
    (void)ctx;

    const char *param_name = esp_rmaker_param_get_name(param);

    ESP_LOGI(TAG, "Write callback param=%s",
             param_name ? param_name : "unknown");

    if (param == s_power_param)
    {
        bool requested = val.val.b;

        ESP_LOGI(TAG, "RainMaker Power=%d", requested);

        if (requested)
        {
            ESP_LOGI(TAG, "RainMaker -> APP_EVENT_START_REQUESTED");

            if (!app_events_publish_start_requested())
            {
                ESP_LOGE(TAG, "No se pudo publicar START_REQUESTED");
                return ESP_FAIL;
            }
        }
        else
        {
            ESP_LOGI(TAG, "RainMaker -> APP_EVENT_STOP_REQUESTED");

            if (!app_events_publish_stop_requested())
            {
                ESP_LOGE(TAG, "No se pudo publicar STOP_REQUESTED");
                return ESP_FAIL;
            }
        }

        /* Refleja localmente el valor recibido */
        esp_rmaker_param_update_and_report(s_power_param, val);
        return ESP_OK;
    }

    ESP_LOGW(TAG, "Parametro no manejado");
    return ESP_OK;
}

bool rainmaker_adapter_init(void)
{
    ESP_LOGI(TAG, "RainMaker init");

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "NVS init fallo: %s", esp_err_to_name(err));
        return false;
    }

    esp_rmaker_config_t cfg = {
        .enable_time_sync = false,
    };

    esp_rmaker_node_t *node = esp_rmaker_node_init(&cfg, "Ensayo LED", "Control");
    if (!node)
    {
        ESP_LOGE(TAG, "No se pudo crear el nodo RainMaker");
        return false;
    }

    s_device = esp_rmaker_device_create("Ensayo", ESP_RMAKER_DEVICE_SWITCH, NULL);
    if (!s_device)
    {
        ESP_LOGE(TAG, "No se pudo crear el device RainMaker");
        return false;
    }

    esp_rmaker_device_add_cb(s_device, write_cb, NULL);

    s_power_param = esp_rmaker_param_create(
        ESP_RMAKER_DEF_POWER_NAME,
        ESP_RMAKER_PARAM_POWER,
        esp_rmaker_bool(false),
        PROP_FLAG_READ | PROP_FLAG_WRITE);

    if (!s_power_param)
    {
        ESP_LOGE(TAG, "No se pudo crear el parametro Power");
        return false;
    }

    esp_err_t add_param_err = esp_rmaker_device_add_param(s_device, s_power_param);
    if (add_param_err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo agregar el parametro Power: %s",
                 esp_err_to_name(add_param_err));
        return false;
    }

    esp_err_t add_dev_err = esp_rmaker_node_add_device(node, s_device);
    if (add_dev_err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo agregar el device al nodo: %s",
                 esp_err_to_name(add_dev_err));
        return false;
    }

    ESP_LOGI(TAG, "RainMaker iniciando cloud");
    

    esp_rmaker_start();
    return true;
}

void rainmaker_adapter_report_runtime(void)
{
    if (s_power_param == NULL)
    {
        return;
    }

    bool running = app_runtime_is_running();

    esp_rmaker_param_update_and_report(
        s_power_param,
        esp_rmaker_bool(running));

    ESP_LOGI(TAG, "RainMaker sync runtime running=%d", running);
}