#include "app_queue.h"
#include "esp_log.h"

static const char *TAG = "APP_QUEUE";

static QueueHandle_t s_app_queue = NULL;

bool app_queue_init(void)
{
    if (s_app_queue != NULL)
    {
        return true;
    }

    s_app_queue = xQueueCreate(8, sizeof(app_cmd_t));

    if (s_app_queue == NULL)
    {
        ESP_LOGE(TAG, "queue creation failed");
        return false;
    }

    ESP_LOGI(TAG, "queue created");
    return true;
}

QueueHandle_t app_queue_get(void)
{
    return s_app_queue;
}

bool app_queue_send(const app_cmd_t *cmd, TickType_t ticks_to_wait)
{
    if ((s_app_queue == NULL) || (cmd == NULL))
    {
        return false;
    }

    if (xQueueSend(s_app_queue, cmd, ticks_to_wait) == pdPASS)
    {
        ESP_LOGI(TAG, "CMD queued type=%d", cmd->type);
        return true;
    }

    ESP_LOGW(TAG, "queue full");
    return false;
}