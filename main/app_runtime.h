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

typedef enum
{
    APP_DOMAIN_IDLE = 0,
    APP_DOMAIN_RUNNING

} app_domain_status_t;

typedef enum
{
    APP_RESULT_NONE = 0,
    APP_RESULT_FINISHED,
    APP_RESULT_STOPPED,
    APP_RESULT_REJECTED

} app_result_t;

typedef struct
{
    bool running;
    uint32_t remaining;
    app_state_t state;

    app_domain_status_t domain_status;
    app_result_t last_result;

} app_runtime_t;

void app_runtime_init(void);

void app_runtime_set_running(bool running);
bool app_runtime_is_running(void);

void app_runtime_set_remaining(uint32_t remaining);
uint32_t app_runtime_get_remaining(void);

void app_runtime_set_state(app_state_t state);
app_state_t app_runtime_get_state(void);

void app_runtime_set_domain_status(app_domain_status_t status);
app_domain_status_t app_runtime_get_domain_status(void);

void app_runtime_set_last_result(app_result_t result);
app_result_t app_runtime_get_last_result(void);

void app_runtime_get_copy(app_runtime_t *out);

#endif