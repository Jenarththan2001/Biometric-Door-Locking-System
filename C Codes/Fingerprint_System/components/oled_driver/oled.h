#ifndef MAIN_OLED_OLED_H_
#define MAIN_OLED_OLED_H_

#include <stdio.h>
#include "../oled_driver/ssd1306.h"

#ifdef __cplusplus
extern "C" {
#endif

void oled_init();
void oled_show_message(const char *msg);
void oled_show_message_2line(const char *line1, const char *line2);
void oled_set_line(int line, const char *msg);
void oled_clear_line(uint8_t line);

// === Line-specific setters ===
void oled_set_line0(const char *msg);
void oled_set_line1(const char *msg);
void oled_set_line2(const char *msg);
void oled_set_line3(const char *msg);
void oled_set_line4(const char *msg);
void oled_set_line5(const char *msg);
void oled_set_line6(const char *msg);
void oled_set_line7(const char *msg);

// === Line-specific clearers ===
void oled_clear_line0();
void oled_clear_line1();
void oled_clear_line2();
void oled_clear_line3();
void oled_clear_line4();
void oled_clear_line5();
void oled_clear_line6();
void oled_clear_line7();

#ifdef __cplusplus
}
#endif

#endif // MAIN_OLED_OLED_H_
