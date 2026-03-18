#ifndef APP_CMD_H
#define APP_CMD_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    APP_CMD_NONE = 0,
    APP_CMD_START,
    APP_CMD_STOP,
    APP_CMD_APPLY_CONFIG

} app_cmd_type_t;

typedef struct
{
    uint32_t led_blink_time;
    uint32_t led_blink_quantity;
    uint32_t led_color;
    bool save_to_nvs;
    bool start_after_apply;

} app_apply_config_t;

typedef struct
{
    app_cmd_type_t type;

    union
    {
        app_apply_config_t apply_config;

    } data;

} app_cmd_t;

#endif