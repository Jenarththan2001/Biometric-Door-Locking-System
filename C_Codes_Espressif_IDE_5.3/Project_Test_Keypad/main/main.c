// main.c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "keypad.h"

void app_main(void) {
    printf("Keypad Testing...\n");
    keypad_init();

    while (1) {
        char key = keypad_get_key();
        if (key != '\0') {
            printf("Key Press: %c\n", key);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
