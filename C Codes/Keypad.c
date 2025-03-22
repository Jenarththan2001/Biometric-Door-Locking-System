#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define ROWS 4  // Number of rows in the keypad
#define COLS 4  // Number of columns in the keypad

// GPIO pin assignments for keypad rows and columns
const int row_pins[ROWS] = {13, 12, 14, 27};  // Input pins connected to keypad rows
const int col_pins[COLS] = {26, 25, 33, 32};  // Output pins connected to keypad columns

// Keypad layout map (4x4)
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Initialize GPIO pins for keypad
void keypad_init() {
    // Configure row pins as input with pull-up resistors
    for (int i = 0; i < ROWS; i++) {
        gpio_set_direction(row_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(row_pins[i], GPIO_PULLUP_ONLY);
    }
    // Configure column pins as output and set HIGH initially
    for (int i = 0; i < COLS; i++) {
        gpio_set_direction(col_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(col_pins[i], 1);  // Deactivate column
    }
}

// Scan the keypad and return the character of the pressed key
char read_keypad() {
    for (int c = 0; c < COLS; c++) {
        gpio_set_level(col_pins[c], 0);        // Activate one column at a time (set LOW)
        vTaskDelay(pdMS_TO_TICKS(10));         // Delay to allow signal to stabilize

        for (int r = 0; r < ROWS; r++) {
            // Check if the key at [r][c] is pressed (row reads LOW)
            if (gpio_get_level(row_pins[r]) == 0) {
                while (gpio_get_level(row_pins[r]) == 0);  // Wait for key release (debounce)
                gpio_set_level(col_pins[c], 1);            // Deactivate the column
                return keys[r][c];                         // Return the corresponding character
            }
        }

        gpio_set_level(col_pins[c], 1);  // Deactivate column (set HIGH)
    }
    return '\0';  // Return null character if no key is pressed
}

// FreeRTOS task to continuously scan and print keypad input
void keypad_task(void *pvParameter) {
    while (1) {
        char key = read_keypad();  // Read pressed key
        if (key) {
            printf("Key Pressed: %c\n", key);  // Print key to console
        }
        vTaskDelay(pdMS_TO_TICKS(200));  // Delay to avoid bouncing and excessive CPU usage
    }
}

// Main function
void app_main() {
    keypad_init();  // Initialize keypad GPIO
    xTaskCreate(keypad_task, "keypad_task", 4096, NULL, 5, NULL);  // Start keypad scanning task
}
