#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define GREEN_LED GPIO_NUM_18
#define RED_LED   GPIO_NUM_19

void app_main(void) {
    // Configure both LED GPIOs as outputs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GREEN_LED) | (1ULL << RED_LED),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    while (1) {
        // Green ON, Red OFF
        gpio_set_level(GREEN_LED, 1);
        gpio_set_level(RED_LED, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Green OFF, Red ON
        gpio_set_level(GREEN_LED, 0);
        gpio_set_level(RED_LED, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
