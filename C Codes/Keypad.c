#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define ROWS 4
#define COLS 4

// Define GPIO pins for rows and columns
const int row_pins[ROWS] = {13, 12, 14, 27};  // Rows
const int col_pins[COLS] = {26, 25, 33, 32};  // Columns

// Define Keypad Layout
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Initialize keypad GPIO
void keypad_init() {
    // Set row pins as input with pull-up resistors
    for (int i = 0; i < ROWS; i++) {
        gpio_set_direction(row_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(row_pins[i], GPIO_PULLUP_ONLY);
    }
    // Set column pins as output and default HIGH
    for (int i = 0; i < COLS; i++) {
        gpio_set_direction(col_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(col_pins[i], 1);
    }
}

// Function to read the pressed key
char read_keypad() {
    for (int c = 0; c < COLS; c++) {
        gpio_set_level(col_pins[c], 0);  // Activate column
        vTaskDelay(pdMS_TO_TICKS(10));   // Small delay for stability

        for (int r = 0; r < ROWS; r++) {
            if (gpio_get_level(row_pins[r]) == 0) {  // Key pressed
                while (gpio_get_level(row_pins[r]) == 0);  // Wait for release
                gpio_set_level(col_pins[c], 1);  // Deactivate column
                return keys[r][c];  // Return pressed key
            }
        }
        gpio_set_level(col_pins[c], 1);  // Deactivate column
    }
    return '\0';  // No key pressed
}

// Task to continuously read keypad input
void keypad_task(void *pvParameter) {
    while (1) {
        char key = read_keypad();
        if (key) {
            printf("Key Pressed: %c\n", key);
        }
        vTaskDelay(pdMS_TO_TICKS(200));  // Small delay to prevent bouncing
    }
}

void app_main() {
    keypad_init();
    xTaskCreate(keypad_task, "keypad_task", 4096, NULL, 5, NULL);
}