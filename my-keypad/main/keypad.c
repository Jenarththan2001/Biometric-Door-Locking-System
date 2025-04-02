// keypad.c
#include "keypad.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ROWS 4
#define COLS 4

// Define keypad GPIO pins
static const gpio_num_t row_pins[ROWS] = {13, 12, 14, 27};   // Row 1 - Row 4
static const gpio_num_t col_pins[COLS] = {26, 25, 33, 32};   // Col 1 - Col 4

// Define keypad key map
static const char keymap[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void keypad_init(void) {
    // Initialize row pins as input with pull-up
    for (int i = 0; i < ROWS; i++) {
        gpio_reset_pin(row_pins[i]);
        gpio_set_direction(row_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(row_pins[i], GPIO_PULLUP_ONLY);
    }

    // Initialize column pins as output and set HIGH
    for (int i = 0; i < COLS; i++) {
        gpio_reset_pin(col_pins[i]);
        gpio_set_direction(col_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(col_pins[i], 1);
    }
}

char keypad_get_key(void) {
    for (int c = 0; c < COLS; c++) {
        // Set all columns HIGH
        for (int k = 0; k < COLS; k++) {
            gpio_set_level(col_pins[k], 1);
        }

        // Pull the current column LOW
        gpio_set_level(col_pins[c], 0);
        vTaskDelay(pdMS_TO_TICKS(1));

        // Scan rows
        for (int r = 0; r < ROWS; r++) {
            if (gpio_get_level(row_pins[r]) == 0) {
                // Debounce
                vTaskDelay(pdMS_TO_TICKS(20));
                if (gpio_get_level(row_pins[r]) == 0) {
                    // Wait for release
                    while (gpio_get_level(row_pins[r]) == 0) {
                        vTaskDelay(pdMS_TO_TICKS(5));
                    }
                    return keymap[r][c];
                }
            }
        }
    }
    return '\0'; // No key pressed
}
