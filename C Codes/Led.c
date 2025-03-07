#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_PIN 2  // GPIO2 (Built-in LED on some ESP32 boards)

void app_main() {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_PIN, 1);  // Turn LED ON
        printf("LED ON\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait 1 second

        gpio_set_level(LED_PIN, 0);  // Turn LED OFF
        printf("LED OFF\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait 1 second
    }
}
