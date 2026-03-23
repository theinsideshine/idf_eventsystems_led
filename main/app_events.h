#ifndef APP_EVENTS_H
#define APP_EVENTS_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

ESP_EVENT_DECLARE_BASE(APP_EVENTS);

typedef enum
{
    APP_EVENT_CONFIG_UPDATED = 1,
    APP_EVENT_START_REQUESTED,
    APP_EVENT_STOP_REQUESTED,
    APP_EVENT_ENSAYO_STARTED,
    APP_EVENT_ENSAYO_FINISHED,
    APP_EVENT_ENSAYO_STOPPED

} app_event_id_t;

typedef struct
{
    uint32_t led_blink_time;
    uint32_t led_blink_quantity;
    uint32_t led_color;
    bool save_to_nvs;

} app_event_config_updated_t;

bool app_events_init(void);

bool app_events_publish_config_updated(const app_event_config_updated_t *cfg);
bool app_events_publish_start_requested(void);
bool app_events_publish_stop_requested(void);

bool app_events_publish_ensayo_started(void);
bool app_events_publish_ensayo_finished(void);
bool app_events_publish_ensayo_stopped(void);

#ifdef __cplusplus
}
#endif

#endif