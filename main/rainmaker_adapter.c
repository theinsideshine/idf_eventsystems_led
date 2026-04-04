#include "esp_log.h"
#include "esp_err.h"

/* RainMaker */
#include "esp_rmaker_core.h"
#include "esp_rmaker_standard_types.h"
#include "esp_rmaker_standard_params.h"

#include "app_events.h"
#include "app_runtime.h"
#include "config_manager.h"
#include "rainmaker_adapter.h"

static const char *TAG = "RAINMAKER";

static esp_rmaker_device_t *s_device = NULL;

static esp_rmaker_param_t *s_power_param = NULL;
static esp_rmaker_param_t *s_blink_time_param = NULL;
static esp_rmaker_param_t *s_blink_qty_param = NULL;
static esp_rmaker_param_t *s_led_color_param = NULL;

#define RM_PARAM_BLINK_TIME_NAME   "blink_time"
#define RM_PARAM_BLINK_QTY_NAME    "blink_quantity"
#define RM_PARAM_LED_COLOR_NAME    "led_color"

static bool publish_full_config_event(uint32_t blink_time,
                                      uint32_t blink_qty,
                                      uint32_t led_color,
                                      bool save_to_nvs)
{
    app_event_config_updated_t cfg;

    cfg.led_blink_time = blink_time;
    cfg.led_blink_quantity = blink_qty;
    cfg.led_color = led_color;
    cfg.save_to_nvs = save_to_nvs;

    ESP_LOGI(TAG,
             "RainMaker -> CONFIG_UPDATED t=%lu q=%lu c=%lu save=%d",
             (unsigned long)cfg.led_blink_time,
             (unsigned long)cfg.led_blink_quantity,
             (unsigned long)cfg.led_color,
             cfg.save_to_nvs);

    return app_events_publish_config_updated(&cfg);
}

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
            if (!app_events_publish_start_requested())
            {
                ESP_LOGE(TAG, "No se pudo publicar START_REQUESTED");
                return ESP_FAIL;
            }
        }
        else
        {
            if (!app_events_publish_stop_requested())
            {
                ESP_LOGE(TAG, "No se pudo publicar STOP_REQUESTED");
                return ESP_FAIL;
            }
        }

        esp_rmaker_param_update_and_report(s_power_param, val);
        return ESP_OK;
    }

    if (param == s_blink_time_param)
    {
        uint32_t new_time = (uint32_t)val.val.i;

        if (!publish_full_config_event(new_time,
                                       config_get_led_blink_quantity(),
                                       config_get_led_color(),
                                       true))
        {
            ESP_LOGE(TAG, "No se pudo publicar CONFIG_UPDATED (blink_time)");
            return ESP_FAIL;
        }

        esp_rmaker_param_update_and_report(s_blink_time_param, val);
        return ESP_OK;
    }

    if (param == s_blink_qty_param)
    {
        uint32_t new_qty = (uint32_t)val.val.i;

        if (!publish_full_config_event(config_get_led_blink_time(),
                                       new_qty,
                                       config_get_led_color(),
                                       true))
        {
            ESP_LOGE(TAG, "No se pudo publicar CONFIG_UPDATED (blink_quantity)");
            return ESP_FAIL;
        }

        esp_rmaker_param_update_and_report(s_blink_qty_param, val);
        return ESP_OK;
    }

    if (param == s_led_color_param)
    {
        uint32_t new_color = (uint32_t)val.val.i;

        if (!publish_full_config_event(config_get_led_blink_time(),
                                       config_get_led_blink_quantity(),
                                       new_color,
                                       true))
        {
            ESP_LOGE(TAG, "No se pudo publicar CONFIG_UPDATED (led_color)");
            return ESP_FAIL;
        }

        esp_rmaker_param_update_and_report(s_led_color_param, val);
        return ESP_OK;
    }

    ESP_LOGW(TAG, "Parametro no manejado");
    return ESP_OK;
}

bool rainmaker_adapter_init(void)
{
    ESP_LOGI(TAG, "RainMaker init");

    esp_rmaker_config_t cfg = {
        .enable_time_sync = false,
    };

    esp_rmaker_node_t *node = esp_rmaker_node_init(&cfg, "Ensayo LED", "Control");
    if (!node)
    {
        ESP_LOGE(TAG, "No se pudo crear el nodo RainMaker");
        return false;
    }

    /*
     * Device CUSTOM:
     * no usar ESP_RMAKER_DEVICE_SWITCH para este caso,
     * porque queremos exponer params propios de configuracion.
     */
    s_device = esp_rmaker_device_create("Ensayo", NULL, NULL);
    if (!s_device)
    {
        ESP_LOGE(TAG, "No se pudo crear el device RainMaker");
        return false;
    }

    esp_rmaker_device_add_cb(s_device, write_cb, NULL);

    /* Power */
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

    esp_rmaker_param_add_ui_type(s_power_param, ESP_RMAKER_UI_TOGGLE);

    /* blink_time */
    s_blink_time_param = esp_rmaker_param_create(
        RM_PARAM_BLINK_TIME_NAME,
        NULL,
        esp_rmaker_int((int)config_get_led_blink_time()),
        PROP_FLAG_READ | PROP_FLAG_WRITE);

    if (!s_blink_time_param)
    {
        ESP_LOGE(TAG, "No se pudo crear el parametro blink_time");
        return false;
    }

    esp_rmaker_param_add_ui_type(s_blink_time_param, ESP_RMAKER_UI_SLIDER);
    esp_rmaker_param_add_bounds(s_blink_time_param,
                                esp_rmaker_int(100),
                                esp_rmaker_int(5000),
                                esp_rmaker_int(10));

    /* blink_quantity */
    s_blink_qty_param = esp_rmaker_param_create(
        RM_PARAM_BLINK_QTY_NAME,
        NULL,
        esp_rmaker_int((int)config_get_led_blink_quantity()),
        PROP_FLAG_READ | PROP_FLAG_WRITE);

    if (!s_blink_qty_param)
    {
        ESP_LOGE(TAG, "No se pudo crear el parametro blink_quantity");
        return false;
    }

    esp_rmaker_param_add_ui_type(s_blink_qty_param, ESP_RMAKER_UI_SLIDER);
    esp_rmaker_param_add_bounds(s_blink_qty_param,
                                esp_rmaker_int(1),
                                esp_rmaker_int(20),
                                esp_rmaker_int(1));

    /* led_color */
    s_led_color_param = esp_rmaker_param_create(
        RM_PARAM_LED_COLOR_NAME,
        NULL,
        esp_rmaker_int((int)config_get_led_color()),
        PROP_FLAG_READ | PROP_FLAG_WRITE);

    if (!s_led_color_param)
    {
        ESP_LOGE(TAG, "No se pudo crear el parametro led_color");
        return false;
    }

    /*
     * Lo dejamos como slider entero para no tocar tu modelo actual.
     * Mas adelante, si queres, lo pasamos a string + dropdown.
     */
    esp_rmaker_param_add_ui_type(s_led_color_param, ESP_RMAKER_UI_SLIDER);
    esp_rmaker_param_add_bounds(s_led_color_param,
                                esp_rmaker_int(1),
                                esp_rmaker_int(10),
                                esp_rmaker_int(1));

    esp_err_t err;

    err = esp_rmaker_device_add_param(s_device, s_power_param);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo agregar Power: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_rmaker_device_add_param(s_device, s_blink_time_param);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo agregar blink_time: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_rmaker_device_add_param(s_device, s_blink_qty_param);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo agregar blink_quantity: %s", esp_err_to_name(err));
        return false;
    }

    err = esp_rmaker_device_add_param(s_device, s_led_color_param);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo agregar led_color: %s", esp_err_to_name(err));
        return false;
    }

    esp_rmaker_device_assign_primary_param(s_device, s_power_param);

    err = esp_rmaker_node_add_device(node, s_device);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "No se pudo agregar el device al nodo: %s",
                 esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG,
             "RainMaker cfg inicial t=%lu q=%lu c=%lu",
             (unsigned long)config_get_led_blink_time(),
             (unsigned long)config_get_led_blink_quantity(),
             (unsigned long)config_get_led_color());

    err = esp_rmaker_start();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_rmaker_start fallo: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGI(TAG, "RainMaker iniciado");
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

void rainmaker_adapter_report_config(uint32_t blink_time,
                                     uint32_t blink_quantity,
                                     uint32_t led_color)
{
    if ((s_blink_time_param == NULL) ||
        (s_blink_qty_param == NULL) ||
        (s_led_color_param == NULL))
    {
        return;
    }

    esp_rmaker_param_update_and_report(
        s_blink_time_param,
        esp_rmaker_int((int)blink_time));

    esp_rmaker_param_update_and_report(
        s_blink_qty_param,
        esp_rmaker_int((int)blink_quantity));

    esp_rmaker_param_update_and_report(
        s_led_color_param,
        esp_rmaker_int((int)led_color));

    ESP_LOGI(TAG,
             "RainMaker sync cfg t=%lu q=%lu c=%lu",
             (unsigned long)blink_time,
             (unsigned long)blink_quantity,
             (unsigned long)led_color);
}