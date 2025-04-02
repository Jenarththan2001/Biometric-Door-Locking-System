#ifndef MAIN_OLED_OLED_H_
#define MAIN_OLED_OLED_H_

#include <stdio.h>
#include "ssd1306.h"

#ifdef __cplusplus
extern "C" {
#endif

void oled_init();
void oled_show_message(const char *msg);
void oled_show_message_2line(const char *line1, const char *line2);

#ifdef __cplusplus
}
#endif

#endif // MAIN_OLED_OLED_H_
