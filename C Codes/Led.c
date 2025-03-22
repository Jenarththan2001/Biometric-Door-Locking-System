#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define the GPIO pin for the LED
#define LED_PIN 2  // GPIO2 (often connected to onboard LED on ESP32 dev boards)

void app_main() {
    // Reset the LED pin (clears previous configuration)
    gpio_reset_pin(LED_PIN);

    // Set the LED pin as output
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Infinite loop to blink the LED
    while (1) {
        gpio_set_level(LED_PIN, 1);  // Turn LED ON
        printf("LED ON\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second

        gpio_set_level(LED_PIN, 0);  // Turn LED OFF
        printf("LED OFF\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for 1 second
    }
}
