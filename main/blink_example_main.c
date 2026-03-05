#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_control.h"
#include "timer_control.h"

void app_main(void)
{
    // Instanciamos las estructuras (la memoria para nuestros módulos)
    my_led_t status_led; 
    my_timer_t blink_timer;
    bool led_state = false;

    // Inicialización: Configuramos el LED en el GPIO 48 y elegimos color
    led_init(&status_led, 48);          
    led_set_color(&status_led, COLOR_GREEN); 
    timer_start(&blink_timer);          

    printf("Sistema iniciado. Parpadeo no bloqueante activo...\n");

    while (1) {
        // Consultamos el timer: ¿Pasaron 500ms?
        if (timer_expired(&blink_timer, 500)) { 
            timer_start(&blink_timer); // Reiniciamos el cronómetro
            
            led_state = !led_state;
            
            if (led_state) {
                led_on(&status_led);
                printf("LED ON\n");
            } else {
                led_off(&status_led);
                printf("LED OFF\n");
            }
        }

        // El pequeño delay para que el perro guardián (Watchdog) esté feliz
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}