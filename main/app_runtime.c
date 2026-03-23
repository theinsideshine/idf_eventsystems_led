#include <stddef.h>
#include "app_runtime.h"

static app_runtime_t s_runtime;

void app_runtime_init(void)
{
    s_runtime.running = false;
    s_runtime.remaining = 0;
    s_runtime.state = APP_STATE_INIT;

    s_runtime.domain_status = APP_DOMAIN_IDLE;
    s_runtime.last_result = APP_RESULT_NONE;
}

void app_runtime_set_running(bool running)
{
    s_runtime.running = running;
}

bool app_runtime_is_running(void)
{
    return s_runtime.running;
}

void app_runtime_set_remaining(uint32_t remaining)
{
    s_runtime.remaining = remaining;
}

uint32_t app_runtime_get_remaining(void)
{
    return s_runtime.remaining;
}

void app_runtime_set_state(app_state_t state)
{
    s_runtime.state = state;
}

app_state_t app_runtime_get_state(void)
{
    return s_runtime.state;
}

void app_runtime_set_domain_status(app_domain_status_t status)
{
    s_runtime.domain_status = status;
}

app_domain_status_t app_runtime_get_domain_status(void)
{
    return s_runtime.domain_status;
}

void app_runtime_set_last_result(app_result_t result)
{
    s_runtime.last_result = result;
}

app_result_t app_runtime_get_last_result(void)
{
    return s_runtime.last_result;
}

void app_runtime_get_copy(app_runtime_t *out)
{
    if (out == NULL)
    {
        return;
    }

    *out = s_runtime;
}