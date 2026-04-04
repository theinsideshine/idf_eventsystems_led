#ifndef RAINMAKER_ADAPTER_H
#define RAINMAKER_ADAPTER_H

#include <stdbool.h>
#include <stdint.h>

bool rainmaker_adapter_init(void);
void rainmaker_adapter_report_runtime(void);
void rainmaker_adapter_report_config(uint32_t blink_time,
                                     uint32_t blink_quantity,
                                     uint32_t led_color);

#endif