#ifndef APP_QUEUE_H
#define APP_QUEUE_H

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "app_cmd.h"

bool app_queue_init(void);
QueueHandle_t app_queue_get(void);
bool app_queue_send(const app_cmd_t *cmd, TickType_t ticks_to_wait);

#endif