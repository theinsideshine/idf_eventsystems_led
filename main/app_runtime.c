#include "app_runtime.h"
#include <stddef.h>

static app_runtime_t s_runtime;

void app_runtime_init(void)
{
    s_runtime.running = false;
    s_runtime.state = APP_STATE_INIT;
    s_runtime.remaining = 0;
}

bool app_runtime_is_running(void)
{
    return s_runtime.running;
}

void app_runtime_set_running(bool value)
{
    s_runtime.running = value;
}

app_state_t app_runtime_get_state(void)
{
    return s_runtime.state;
}

void app_runtime_set_state(app_state_t state)
{
    s_runtime.state = state;
}

uint32_t app_runtime_get_remaining(void)
{
    return s_runtime.remaining;
}

void app_runtime_set_remaining(uint32_t value)
{
    s_runtime.remaining = value;
}

void app_runtime_get_copy(app_runtime_t *runtime)
{
    if (runtime != NULL)
    {
        *runtime = s_runtime;
    }
}