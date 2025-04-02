#include "oled.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "OLED";
SSD1306_t dev;

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST -1

void oled_init() {
    ESP_LOGI(TAG, "Initializing OLED...");
    i2c_master_init(&dev, OLED_SDA, OLED_SCL, OLED_RST);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_contrast(&dev, 0xFF);
    ESP_LOGI(TAG, "OLED Initialized");
}

void oled_show_message(const char *msg) {
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 3, msg, strlen(msg), false);
}

void oled_show_message_2line(const char *line1, const char *line2) {
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 2, line1, strlen(line1), false);
    ssd1306_display_text(&dev, 4, line2, strlen(line2), false);
}
