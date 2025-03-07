#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "ssd1306.h"

#define I2C_MASTER_SDA 21  // Define SDA Pin
#define I2C_MASTER_SCL 22  // Define SCL Pin
#define I2C_MASTER_NUM I2C_NUM_0  // I2C Port
#define OLED_ADDR 0x3C  // SSD1306 I2C Address

void init_i2c() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA,
        .scl_io_num = I2C_MASTER_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000  // 400kHz I2C Speed
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

void app_main() {
    init_i2c();
    ssd1306_t oled;
    ssd1306_init(&oled, OLED_ADDR, I2C_MASTER_NUM);

    ssd1306_clear_screen(&oled, false);
    ssd1306_draw_string(&oled, 20, 25, "Welcome", FONT_FACE_6x8, false);
    ssd1306_refresh(&oled, true);
}
