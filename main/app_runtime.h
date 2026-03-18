#ifndef APP_RUNTIME_H
#define APP_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    APP_STATE_INIT = 0,
    APP_STATE_IDLE,
    APP_STATE_LED_ON,
    APP_STATE_LED_OFF

} app_state_t;

typedef struct
{
    bool running;
    uint32_t remaining;
    app_state_t state;

} app_runtime_t;

void app_runtime_init(void);

bool app_runtime_is_running(void);
void app_runtime_set_running(bool running);

uint32_t app_runtime_get_remaining(void);
void app_runtime_set_remaining(uint32_t remaining);

app_state_t app_runtime_get_state(void);
void app_runtime_set_state(app_state_t state);

void app_runtime_get_copy(app_runtime_t *out);

#endif