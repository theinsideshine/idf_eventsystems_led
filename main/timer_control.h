#ifndef TIMER_CONTROL_H
#define TIMER_CONTROL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    TickType_t start_time;
} my_timer_t;

static inline void timer_start(my_timer_t *t) {
    t->start_time = xTaskGetTickCount();
}

static inline bool timer_expired(my_timer_t *t, uint32_t ms) {
    return (xTaskGetTickCount() - t->start_time) >= pdMS_TO_TICKS(ms);
}

#endif