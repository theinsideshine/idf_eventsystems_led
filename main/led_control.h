#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdint.h>
#include "led_strip.h"

// IDs de colores para la lógica de la aplicación
typedef enum {
    COLOR_OFF = 0, COLOR_RED, COLOR_YELLOW, COLOR_GREEN, 
    COLOR_WHITE, COLOR_BLUE, COLOR_CYAN, COLOR_MAGENTA, 
    COLOR_ORANGE, COLOR_VIOLET, COLOR_GRAY
} led_color_t;

// Estructura que mantiene el estado del LED
typedef struct {
    led_strip_handle_t handle;
    uint8_t current_rgb[3];
} my_led_t;

void led_init(my_led_t *led, int pin);
void led_set_color(my_led_t *led, led_color_t color_id);
void led_on(my_led_t *led);
void led_off(my_led_t *led);

#endif