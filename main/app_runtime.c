#include "app_runtime.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static app_runtime_t s_runtime;
static portMUX_TYPE s_runtime_mux = portMUX_INITIALIZER_UNLOCKED;

void app_runtime_init(void)
{
    taskENTER_CRITICAL(&s_runtime_mux);
    s_runtime.running = false;
    s_runtime.remaining = 0;
    s_runtime.state = APP_STATE_INIT;
    taskEXIT_CRITICAL(&s_runtime_mux);
}

bool app_runtime_is_running(void)
{
    bool value;

    taskENTER_CRITICAL(&s_runtime_mux);
    value = s_runtime.running;
    taskEXIT_CRITICAL(&s_runtime_mux);

    return value;
}

void app_runtime_set_running(bool running)
{
    taskENTER_CRITICAL(&s_runtime_mux);
    s_runtime.running = running;
    taskEXIT_CRITICAL(&s_runtime_mux);
}

uint32_t app_runtime_get_remaining(void)
{
    uint32_t value;

    taskENTER_CRITICAL(&s_runtime_mux);
    value = s_runtime.remaining;
    taskEXIT_CRITICAL(&s_runtime_mux);

    return value;
}

void app_runtime_set_remaining(uint32_t remaining)
{
    taskENTER_CRITICAL(&s_runtime_mux);
    s_runtime.remaining = remaining;
    taskEXIT_CRITICAL(&s_runtime_mux);
}

app_state_t app_runtime_get_state(void)
{
    app_state_t value;

    taskENTER_CRITICAL(&s_runtime_mux);
    value = s_runtime.state;
    taskEXIT_CRITICAL(&s_runtime_mux);

    return value;
}

void app_runtime_set_state(app_state_t state)
{
    taskENTER_CRITICAL(&s_runtime_mux);
    s_runtime.state = state;
    taskEXIT_CRITICAL(&s_runtime_mux);
}

void app_runtime_get_copy(app_runtime_t *out)
{
    if (out == NULL)
    {
        return;
    }

    taskENTER_CRITICAL(&s_runtime_mux);
    *out = s_runtime;
    taskEXIT_CRITICAL(&s_runtime_mux);
}