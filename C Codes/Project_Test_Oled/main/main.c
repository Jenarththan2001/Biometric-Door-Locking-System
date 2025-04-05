#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"

// ====== Function Declarations ======
void menu();
void admin_menu();

// ====== app_main ======
void app_main() {
    oled_init();
    oled_show_message("System Ready");
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    menu();
}

// ====== Simple Menu ======
void menu() {
    while (1) {
        oled_show_message("Press # for Admin");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        // Simulated keypad input
        // Replace this with actual keypad read when you integrate keypad
        char key = '#'; // Simulating someone pressing #

        if (key == '#') {
            admin_menu();
        } else {
            oled_show_message("Waiting...");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

// ====== Admin Menu ======
void admin_menu() {
    oled_show_message("Admin Mode");

    // Simulate password check
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    oled_show_message_2line("PWD OK", "1:Enroll 2:Del");

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // Simulated option
    char opt = '1'; // Imagine you pressed '1'

    if (opt == '1') {
        oled_show_message("Enroll Finger");
    } else if (opt == '2') {
        oled_show_message("Delete Finger");
    } else if (opt == '3') {
        oled_show_message("Exit Admin");
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
}
