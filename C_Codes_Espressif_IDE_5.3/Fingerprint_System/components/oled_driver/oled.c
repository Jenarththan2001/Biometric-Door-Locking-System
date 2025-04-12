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

void oled_set_line(int line, const char *msg) {
    ssd1306_display_text(&dev, line, msg, strlen(msg), false);
}

void oled_clear_line(uint8_t line) {
    char empty[17];
    memset(empty, ' ', 16);  // Fill with spaces
    empty[16] = '\0';        // Null-terminate
    ssd1306_display_text(&dev, line, empty, 16, false);
}

// === Individual Line Setters ===
void oled_set_line0(const char *msg) { oled_set_line(0, msg); }
void oled_set_line1(const char *msg) { oled_set_line(1, msg); }
void oled_set_line2(const char *msg) { oled_set_line(2, msg); }
void oled_set_line3(const char *msg) { oled_set_line(3, msg); }
void oled_set_line4(const char *msg) { oled_set_line(4, msg); }
void oled_set_line5(const char *msg) { oled_set_line(5, msg); }
void oled_set_line6(const char *msg) { oled_set_line(6, msg); }
void oled_set_line7(const char *msg) { oled_set_line(7, msg); }

// === Individual Line Clearers ===
void oled_clear_line0() { oled_clear_line(0); }
void oled_clear_line1() { oled_clear_line(1); }
void oled_clear_line2() { oled_clear_line(2); }
void oled_clear_line3() { oled_clear_line(3); }
void oled_clear_line4() { oled_clear_line(4); }
void oled_clear_line5() { oled_clear_line(5); }
void oled_clear_line6() { oled_clear_line(6); }
void oled_clear_line7() { oled_clear_line(7); }
