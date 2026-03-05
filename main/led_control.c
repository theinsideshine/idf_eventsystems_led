#include "led_control.h"

static const uint8_t COLORS[11][3] = {
    {0,0,0}, {255,0,0}, {255,255,0}, {0,255,0}, {255,255,255},
    {0,0,255}, {0,255,255}, {255,0,255}, {255,165,0}, {238,130,238}, {169,169,169}
};

void led_init(my_led_t *led, int pin) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = pin,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    led_strip_new_rmt_device(&strip_config, &rmt_config, &led->handle);
}

void led_set_color(my_led_t *led, led_color_t id) {
    if (id < 11) {
        led->current_rgb[0] = COLORS[id][0];
        led->current_rgb[1] = COLORS[id][1];
        led->current_rgb[2] = COLORS[id][2];
    }
}

void led_on(my_led_t *led) {
    led_strip_set_pixel(led->handle, 0, led->current_rgb[0], led->current_rgb[1], led->current_rgb[2]);
    led_strip_refresh(led->handle);
}

void led_off(my_led_t *led) {
    led_strip_clear(led->handle);
}