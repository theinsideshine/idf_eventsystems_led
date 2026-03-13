#ifndef APP_RUNTIME_H
#define APP_RUNTIME_H

#include <stdint.h>
#include <stdbool.h>

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
    app_state_t state;
    uint32_t remaining;

} app_runtime_t;

// Inicializa el estado runtime del sistema
void app_runtime_init(void);

// Flag principal de ejecución del ensayo
bool app_runtime_is_running(void);
void app_runtime_set_running(bool value);

// Estado actual de la FSM
app_state_t app_runtime_get_state(void);
void app_runtime_set_state(app_state_t state);

// Cantidad de pulsos restantes
uint32_t app_runtime_get_remaining(void);
void app_runtime_set_remaining(uint32_t value);

// Copia completa del runtime
void app_runtime_get_copy(app_runtime_t *runtime);

#endif