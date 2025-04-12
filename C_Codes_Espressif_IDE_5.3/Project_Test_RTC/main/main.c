#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ds1307.h"

void app_main(void) {
    ds1307_init();
    printf("DS1307 Init Done\n");

    // Set the time only once. Comment this line after initial setup.
    if (ds1307_set_time(10, 20, 30)) {
        printf("Time set successfully\n");
    } else {
        printf("Failed to set time\n");
    }

    while (1) {
        uint8_t hour, min, sec;
        if (ds1307_get_time(&hour, &min, &sec)) {
            printf("Time: %02d:%02d:%02d\n", hour, min, sec);
        } else {
            printf("Failed to read time\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));  // Delay 1 second
    }
}