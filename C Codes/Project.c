#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "ssd1306.h"  // Include SSD1306 OLED Library
#include "fingerprint.h" // Include Fingerprint Sensor Library

#define GREEN_LED GPIO_NUM_5
#define RED_LED GPIO_NUM_18

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

#define UART_NUM UART_NUM_2
#define TXD_PIN GPIO_NUM_17
#define RXD_PIN GPIO_NUM_16
#define BUF_SIZE (1024)

// Keypad Configuration
#define ROWS 4
#define COLS 4

const gpio_num_t row_pins[ROWS] = {GPIO_NUM_13, GPIO_NUM_12, GPIO_NUM_14, GPIO_NUM_27};
const gpio_num_t col_pins[COLS] = {GPIO_NUM_26, GPIO_NUM_25, GPIO_NUM_33, GPIO_NUM_32};

char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// OLED Display Object
SSD1306_t oled;

// Fingerprint Sensor Data
static uint8_t fingerprint_buffer[BUF_SIZE];

// Admin Mode
bool inAdminMode = false;

// Function Prototypes
void keypad_init();
char read_keypad();
void display_message(const char *message);
void fingerprint_init();
int fingerprint_search();
void enroll_fingerprint(int id);
void delete_fingerprint(int id);
int get_id_from_keypad();

void app_main() {
    // Initialize Hardware
    printf("Initializing System...\n");
    gpio_set_direction(GREEN_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(RED_LED, GPIO_MODE_OUTPUT);

    keypad_init();
    fingerprint_init();

    // Initialize OLED Display
    i2c_master_init(&oled, I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    ssd1306_init(&oled);
    display_message("System Ready");

    // Main Loop
    while (1) {
        char key = read_keypad();
        if (key == '#') {
            inAdminMode = true;
            display_message("Enter Admin PWD");
            int password = get_id_from_keypad();
            if (password == 123) {
                display_message("Admin Mode");
                printf("Admin Mode Activated\n");
                enroll_fingerprint(get_id_from_keypad());
                delete_fingerprint(get_id_from_keypad());
            } else {
                display_message("Wrong Password!");
                inAdminMode = false;
            }
        }

        if (!inAdminMode) {
            int result = fingerprint_search();
            if (result > 0) {
                printf("Access Granted! ID: %d\n", result);
                display_message("Access Granted!");
                gpio_set_level(GREEN_LED, 1);
                gpio_set_level(RED_LED, 0);
            } else {
                printf("Access Denied!\n");
                display_message("Access Denied!");
                gpio_set_level(GREEN_LED, 0);
                gpio_set_level(RED_LED, 1);
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
            gpio_set_level(GREEN_LED, 0);
            gpio_set_level(RED_LED, 0);
        }
    }
}

// Function to Initialize Keypad
void keypad_init() {
    for (int i = 0; i < ROWS; i++) {
        gpio_set_direction(row_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(row_pins[i], GPIO_PULLUP_ONLY);
    }
    for (int i = 0; i < COLS; i++) {
        gpio_set_direction(col_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(col_pins[i], 1);
    }
}

// Function to Read Keypad Input
char read_keypad() {
    for (int c = 0; c < COLS; c++) {
        gpio_set_level(col_pins[c], 0);
        vTaskDelay(pdMS_TO_TICKS(10));

        for (int r = 0; r < ROWS; r++) {
            if (gpio_get_level(row_pins[r]) == 0) {
                while (gpio_get_level(row_pins[r]) == 0);
                gpio_set_level(col_pins[c], 1);
                return keys[r][c];
            }
        }
        gpio_set_level(col_pins[c], 1);
    }
    return '\0';
}

// Function to Display Message on OLED
void display_message(const char *message) {
    ssd1306_clear_screen(&oled, false);
    ssd1306_display_text(&oled, 0, message, strlen(message), false);
}

// Function to Initialize Fingerprint Sensor
void fingerprint_init() {
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    printf("Initializing Fingerprint Sensor...\n");
}

// Function to Search for a Fingerprint
int fingerprint_search() {
    uart_write_bytes(UART_NUM, "search", 6);
    vTaskDelay(pdMS_TO_TICKS(500));
    int len = uart_read_bytes(UART_NUM, fingerprint_buffer, BUF_SIZE, 1000 / portTICK_PERIOD_MS);
    if (len > 0) {
        return fingerprint_buffer[0];  // Return ID
    }
    return -1;
}

// Function to Enroll a New Fingerprint
void enroll_fingerprint(int id) {
    printf("Place Finger for Enrollment ID: %d\n", id);
    display_message("Place Finger");

    uart_write_bytes(UART_NUM, "enroll", 6);
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("Remove Finger...\n");
    display_message("Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("Place Same Finger Again...\n");
    display_message("Place Again");

    uart_write_bytes(UART_NUM, "confirm", 7);
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("Fingerprint Stored Successfully!\n");
    display_message("Enroll Success!");
}

// Function to Delete a Fingerprint
void delete_fingerprint(int id) {
    printf("Deleting Fingerprint ID: %d\n", id);
    display_message("Deleting ID");
    
    uart_write_bytes(UART_NUM, "delete", 6);
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("Fingerprint Deleted!\n");
    display_message("Delete Success!");
}

// Function to Get an ID from Keypad
int get_id_from_keypad() {
    int id = 0;
    char key;
    while (1) {
        key = read_keypad();
        if (key >= '0' && key <= '9') {
            id = id * 10 + (key - '0');
            display_message("ID: ");
            printf("ID Entered: %d\n", id);
        } else if (key == '#') {
            return id;
        }
    }
}
