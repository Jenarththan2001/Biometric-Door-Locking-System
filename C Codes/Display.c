#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "ssd1306.h"

// Define I2C pins and configuration
#define I2C_MASTER_SDA 21          // SDA pin for I2C communication
#define I2C_MASTER_SCL 22          // SCL pin for I2C communication
#define I2C_MASTER_NUM I2C_NUM_0   // I2C port number
#define OLED_ADDR 0x3C             // I2C address of the SSD1306 OLED display

// Function to initialize I2C in master mode
void init_i2c() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,               // Set mode to master
        .sda_io_num = I2C_MASTER_SDA,          // Assign SDA pin
        .scl_io_num = I2C_MASTER_SCL,          // Assign SCL pin
        .sda_pullup_en = GPIO_PULLUP_ENABLE,   // Enable pull-up on SDA
        .scl_pullup_en = GPIO_PULLUP_ENABLE,   // Enable pull-up on SCL
        .master.clk_speed = 400000             // Set I2C clock speed to 400kHz
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);   // Apply configuration to I2C port
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);  // Install I2C driver
}

// Main application function
void app_main() {
    init_i2c();  // Initialize I2C communication

    ssd1306_t oled;  // Create an instance of the OLED display

    // Initialize the OLED display using its I2C address and I2C port
    ssd1306_init(&oled, OLED_ADDR, I2C_MASTER_NUM);

    // Clear the OLED screen
    ssd1306_clear_screen(&oled, false);

    // Draw the text "Welcome" at coordinates (20, 25) using 6x8 font
    ssd1306_draw_string(&oled, 20, 25, "Welcome", FONT_FACE_6x8, false);

    // Refresh the OLED display to show the changes
    ssd1306_refresh(&oled, true);
}
