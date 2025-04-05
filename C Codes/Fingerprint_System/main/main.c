/*
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"

void menu();
void admin_menu();

void app_main() {
    oled_init();
    oled_show_message("System Ready");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    menu();
}

void menu() {
    while (1) {
        oled_show_message("Press # for Admin");
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        char key = '#'; // Simulated input

        if (key == '#') {
            admin_menu();
        } else {
            oled_show_message("Waiting...");
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

void admin_menu() {
    oled_show_message("Admin Mode");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    oled_show_message_2line("PWD OK", "1:Enroll 2:Del");
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    char opt = '1'; // Simulated

    if (opt == '1') {
        oled_show_message("Enroll Finger");
    } else if (opt == '2') {
        oled_show_message("Delete Finger");
    } else if (opt == '3') {
        oled_show_message("Exit Admin");
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

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
            printf("Key Pressed: %c\n", key);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

*/
/*
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oled.h"
#include "keypad.h"

void app_main() {
    // Initialize OLED
    oled_init();
    oled_show_message("OLED + KEYPAD OK");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Initialize Keypad
    keypad_init();
    oled_show_message("Press Any Key");

    while (1) {
        char key = keypad_get_key();
        
        if (key != '\0') {
            // Prepare message to display
            char msg[20];
            snprintf(msg, sizeof(msg), "Key Pressed: %c", key);
            oled_show_message(msg);

            // Hold the message for 1.5 seconds
            vTaskDelay(pdMS_TO_TICKS(1500));

            // Show ready message again
            oled_show_message("Press Any Key");
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Polling delay
    }
}
*/
/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as608.h"

#define UART_NUM UART_NUM_2   // AS608 UART
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

static const char *TAG = "AS608_MAIN";

// ----------------------
// Fingerprint Enroll
// ----------------------
void enroll_fingerprint(uint16_t pageID)
{
    uint8_t status;
    ESP_LOGI(TAG, "=== Fingerprint Enrollment Start ===");

    // Capture 1
    ESP_LOGI(TAG, "Place your finger (1st capture)");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "Image captured (1st)");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "GenChar1 failed: %s", AS608_StatusString(status));
        return;
    }

    // Wait for finger removal
    ESP_LOGI(TAG, "Remove your finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Capture 2
    ESP_LOGI(TAG, "Place the same finger again (2nd capture)");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "Image captured (2nd)");

    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "GenChar2 failed: %s", AS608_StatusString(status));
        return;
    }

    // Merge features
    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "RegModel failed: %s", AS608_StatusString(status));
        return;
    }

    // Store to page
    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "Store failed: %s", AS608_StatusString(status));
        return;
    }

    ESP_LOGI(TAG, "Fingerprint enrolled successfully at ID: %d", pageID);
}

// ----------------------
// Fingerprint Search
// ----------------------
void search_fingerprint()
{
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    ESP_LOGI(TAG, "Place your finger to search...");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "Image captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "GenChar failed: %s", AS608_StatusString(status));
        return;
    }

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);

    if (status == FINGERPRINT_OK) {
        ESP_LOGI(TAG, "Match Found! ID = %d | Score = %d", pageID, score);
    } else if (status == FINGERPRINT_NOTFOUND || status == FINGERPRINT_NOMATCH) {
        ESP_LOGI(TAG, "No matching fingerprint found");
    } else {
        ESP_LOGW(TAG, "Search failed: %s", AS608_StatusString(status));
    }
}

// ----------------------
// Fingerprint Delete
// ----------------------
void delete_fingerprint(uint16_t pageID)
{
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);

    if (status == FINGERPRINT_OK) {
        ESP_LOGI(TAG, "Fingerprint deleted successfully at ID: %d", pageID);
    } else {
        ESP_LOGE(TAG, "Delete failed: %s", AS608_StatusString(status));
    }
}

// ----------------------
// Main Application
// ----------------------
void app_main(void)
{
    // UART2 --> Fingerprint Sensor
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "UART2 initialized for AS608");

    // UART0 --> Console
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Verify password
    ESP_LOGI(TAG, "Verifying sensor password...");
    if (!AS608_VerifyPassword(UART_NUM)) {
        ESP_LOGE(TAG, "Fingerprint sensor not found or wrong password");
        return;
    }
    ESP_LOGI(TAG, "Fingerprint sensor ready!");

    // ----------- MENU LOOP -----------
    while (1) {
        printf("\n===================\n");
        printf("1 = Enroll Fingerprint\n");
        printf("2 = Search Fingerprint\n");
        printf("3 = Delete Fingerprint\n");
        printf("===================\n");
        printf("Enter choice (1, 2 or 3): ");
        fflush(stdout);

        uint8_t input = 0;
        uart_read_bytes(UART_NUM_0, &input, 1, portMAX_DELAY);
        printf("%c\n", input);

        uint8_t dummy;
        while (uart_read_bytes(UART_NUM_0, &dummy, 1, 10 / portTICK_PERIOD_MS) > 0);

        if (input == '1' || input == '3') {
            printf("Enter Fingerprint ID (0 - 127): ");
            fflush(stdout);

            char id_input[4] = {0};
            int idx = 0;
            while (1) {
                char ch = 0;
                uart_read_bytes(UART_NUM_0, (uint8_t *)&ch, 1, portMAX_DELAY);
                if (ch == '\r' || ch == '\n') break;

                if ((ch == 8 || ch == 127) && idx > 0) {
                    idx--;
                    printf("\b \b");
                    fflush(stdout);
                    continue;
                }
                if (ch >= '0' && ch <= '9' && idx < 3) {
                    id_input[idx++] = ch;
                    printf("%c", ch);
                    fflush(stdout);
                }
            }
            id_input[idx] = '\0';
            uint16_t pageID = atoi(id_input);
            printf("\n");

            if (pageID > 127) {
                printf("Invalid ID! Must be between 0 and 127.\n");
                continue;
            }

            if (input == '1')
                enroll_fingerprint(pageID);
            else
                delete_fingerprint(pageID);
        }
        else if (input == '2') {
            search_fingerprint();
        }
        else {
            printf("Invalid choice! Enter 1, 2 or 3.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


*/

/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as608.h"
#include "oled.h" 

#define UART_NUM UART_NUM_2   // AS608 UART
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

static const char *TAG = "AS608_MAIN";

// ----------------------
// Fingerprint Enroll
// ----------------------
void enroll_fingerprint(uint16_t pageID)
{
    uint8_t status;
    ESP_LOGI(TAG, "=== Fingerprint Enrollment Start ===");

    // Capture 1
    ESP_LOGI(TAG, "Place your finger (1st capture)");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "Image captured (1st)");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "GenChar1 failed: %s", AS608_StatusString(status));
        return;
    }

    // Wait for finger removal
    ESP_LOGI(TAG, "Remove your finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Capture 2
    ESP_LOGI(TAG, "Place the same finger again (2nd capture)");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "Image captured (2nd)");

    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "GenChar2 failed: %s", AS608_StatusString(status));
        return;
    }

    // Merge features
    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "RegModel failed: %s", AS608_StatusString(status));
        return;
    }

    // Store to page
    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "Store failed: %s", AS608_StatusString(status));
        return;
    }

    ESP_LOGI(TAG, "Fingerprint enrolled successfully at ID: %d", pageID);
}

// ----------------------
// Fingerprint Search
// ----------------------
void search_fingerprint()
{
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    ESP_LOGI(TAG, "Place your finger to search...");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    ESP_LOGI(TAG, "Image captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        ESP_LOGE(TAG, "GenChar failed: %s", AS608_StatusString(status));
        return;
    }

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);

    if (status == FINGERPRINT_OK) {
        ESP_LOGI(TAG, "Match Found! ID = %d | Score = %d", pageID, score);
    } else if (status == FINGERPRINT_NOTFOUND || status == FINGERPRINT_NOMATCH) {
        ESP_LOGI(TAG, "No matching fingerprint found");
    } else {
        ESP_LOGW(TAG, "Search failed: %s", AS608_StatusString(status));
    }
}

// ----------------------
// Fingerprint Delete
// ----------------------
void delete_fingerprint(uint16_t pageID)
{
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);

    if (status == FINGERPRINT_OK) {
        ESP_LOGI(TAG, "Fingerprint deleted successfully at ID: %d", pageID);
    } else {
        ESP_LOGE(TAG, "Delete failed: %s", AS608_StatusString(status));
    }
}

// ----------------------
// Main Application
// ----------------------
void app_main(void)
{
    // UART2 --> Fingerprint Sensor
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    ESP_LOGI(TAG, "UART2 initialized for AS608");
    
    oled_init();
    oled_show_message("OLED + Fingerprint");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // UART0 --> Console
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Verify password
    ESP_LOGI(TAG, "Verifying sensor password...");
    if (!AS608_VerifyPassword(UART_NUM)) {
        ESP_LOGE(TAG, "Fingerprint sensor not found or wrong password");
        return;
    }
    ESP_LOGI(TAG, "Fingerprint sensor ready!");

    // ----------- MENU LOOP -----------
    while (1) {
        printf("\n===================\n");
        printf("1 = Enroll Fingerprint\n");
        printf("2 = Search Fingerprint\n");
        printf("3 = Delete Fingerprint\n");
        printf("===================\n");
        printf("Enter choice (1, 2 or 3): ");
        fflush(stdout);

        uint8_t input = 0;
        uart_read_bytes(UART_NUM_0, &input, 1, portMAX_DELAY);
        printf("%c\n", input);

        uint8_t dummy;
        while (uart_read_bytes(UART_NUM_0, &dummy, 1, 10 / portTICK_PERIOD_MS) > 0);

        if (input == '1' || input == '3') {
            printf("Enter Fingerprint ID (0 - 127): ");
            fflush(stdout);

            char id_input[4] = {0};
            int idx = 0;
            while (1) {
                char ch = 0;
                uart_read_bytes(UART_NUM_0, (uint8_t *)&ch, 1, portMAX_DELAY);
                if (ch == '\r' || ch == '\n') break;

                if ((ch == 8 || ch == 127) && idx > 0) {
                    idx--;
                    printf("\b \b");
                    fflush(stdout);
                    continue;
                }
                if (ch >= '0' && ch <= '9' && idx < 3) {
                    id_input[idx++] = ch;
                    printf("%c", ch);
                    fflush(stdout);
                }
            }
            id_input[idx] = '\0';
            uint16_t pageID = atoi(id_input);
            printf("\n");

            if (pageID > 127) {
                printf("Invalid ID! Must be between 0 and 127.\n");
                continue;
            }

            if (input == '1')
                enroll_fingerprint(pageID);
            else
                delete_fingerprint(pageID);
        }
        else if (input == '2') {
            search_fingerprint();
        }
        else {
            printf("Invalid choice! Enter 1, 2 or 3.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as608.h"
#include "oled.h" 

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

static const char *TAG = "AS608_MAIN";

void enroll_fingerprint(uint16_t pageID)
{
    uint8_t status;
    printf("Enroll Start\n");
    oled_show_message("Enroll Start");

    printf("Place finger 1\n");
    oled_show_message("Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    printf("Captured 1\n");
    oled_show_message("Captured 1");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        printf("GenChar1 Failed\n");
        oled_show_message("GenChar1 Fail");
        return;
    }

    printf("Remove finger\n");
    oled_show_message("Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    printf("Place finger 2\n");
    oled_show_message("Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    printf("Captured 2\n");
    oled_show_message("Captured 2");

    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) {
        printf("GenChar2 Failed\n");
        oled_show_message("GenChar2 Fail");
        return;
    }

    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) {
        printf("RegModel Failed\n");
        oled_show_message("RegModel Fail");
        return;
    }

    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) {
        printf("Store Failed\n");
        oled_show_message("Store Fail");
        return;
    }

    printf("Enroll OK: ID %d\n", pageID);
    oled_show_message("Enroll OK");
}

void search_fingerprint()
{
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    printf("Place finger\n");
    oled_show_message("Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    printf("Captured\n");
    oled_show_message("Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        printf("GenChar Fail\n");
        oled_show_message("GenChar Fail");
        return;
    }

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);

    if (status == FINGERPRINT_OK) {
printf("Match: ID=%d Score=%d\n", pageID, score);

char line2[20];
snprintf(line2, sizeof(line2), "ID:%d Sc:%d", pageID, score);
oled_show_message_2line("Match Found", line2);

    } else if (status == FINGERPRINT_NOTFOUND || status == FINGERPRINT_NOMATCH) {
        printf("No Match\n");
        oled_show_message("No Match");
    } else {
        printf("Search Fail\n");
        oled_show_message("Search Fail");
    }
}

void delete_fingerprint(uint16_t pageID)
{
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);

    if (status == FINGERPRINT_OK) {
        printf("Deleted ID: %d\n", pageID);
        oled_show_message("Delete OK");
    } else {
        printf("Delete Fail\n");
        oled_show_message("Delete Fail");
    }
}

void app_main(void)
{
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    oled_init();
    oled_show_message("OLED + FP Ready");
    vTaskDelay(pdMS_TO_TICKS(2000));

    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));

    printf("Verifying sensor...\n");
    oled_show_message("Check Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        printf("Sensor Error\n");
        oled_show_message("Sensor Error");
        return;
    }

    printf("Sensor OK\n");
    oled_show_message("Sensor OK");

    while (1) {
        printf("\n1 = Enroll\n2 = Search\n3 = Delete\n");
        printf("Enter choice: ");
        oled_show_message("1:Enr2:Sea3.Del");

        uint8_t input = 0;
        uart_read_bytes(UART_NUM_0, &input, 1, portMAX_DELAY);
        printf("%c\n", input);

        uint8_t dummy;
        while (uart_read_bytes(UART_NUM_0, &dummy, 1, 10 / portTICK_PERIOD_MS) > 0);

        if (input == '1' || input == '3') {
            printf("Enter ID (0-127): ");
            oled_show_message("Enter ID");
            char id_input[4] = {0}; int idx = 0;

            while (1) {
                char ch = 0;
                uart_read_bytes(UART_NUM_0, (uint8_t *)&ch, 1, portMAX_DELAY);
                if (ch == '\r' || ch == '\n') break;
                if ((ch >= '0' && ch <= '9') && idx < 3) {
                    id_input[idx++] = ch;
                    printf("%c", ch);
                }
            }
            id_input[idx] = '\0';
            uint16_t pageID = atoi(id_input);
            printf("\n");

            if (pageID > 127) {
                printf("Invalid ID\n");
                oled_show_message("Invalid ID");
                continue;
            }

            if (input == '1') enroll_fingerprint(pageID);
            else delete_fingerprint(pageID);
        }
        else if (input == '2') {
            search_fingerprint();
        }
        else {
            printf("Invalid choice!\n");
            oled_show_message("Invalid Opt");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as608.h"
#include "oled.h"
#include "keypad.h"

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

void enroll_fingerprint(uint16_t pageID) {
    uint8_t status;
    oled_show_message("Enroll Start");

    oled_show_message("Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured 1");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        oled_show_message("GenChar1 Fail");
        return;
    }

    oled_show_message("Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    oled_show_message("Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured 2");

    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) {
        oled_show_message("GenChar2 Fail");
        return;
    }

    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) {
        oled_show_message("RegModel Fail");
        return;
    }

    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) {
        oled_show_message("Store Fail");
        return;
    }

    oled_show_message("Enroll OK");
}

void search_fingerprint() {
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    oled_show_message("Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        oled_show_message("GenChar Fail");
        return;
    }

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
    if (status == FINGERPRINT_OK) {
        char line2[20];
        snprintf(line2, sizeof(line2), "ID:%d Sc:%d", pageID, score);
        oled_show_message_2line("Match Found", line2);
    } else if (status == FINGERPRINT_NOTFOUND || status == FINGERPRINT_NOMATCH) {
        oled_show_message("No Match");
    } else {
        oled_show_message("Search Fail");
    }
}

void delete_fingerprint(uint16_t pageID) {
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);
    if (status == FINGERPRINT_OK) {
        oled_show_message("Delete OK");
    } else {
        oled_show_message("Delete Fail");
    }
}

uint16_t get_id_from_keypad() {
    char id_buf[4] = {0};
    int idx = 0;
    oled_show_message("Enter ID (0-127)");

    while (idx < 3) {
        char key = keypad_get_key();
        if (key >= '0' && key <= '9') {
            id_buf[idx++] = key;
            char disp[16];
            snprintf(disp, sizeof(disp), "ID: %s", id_buf);
            oled_show_message(disp);
        } else if (key == '#') {
            break; // Confirm
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return atoi(id_buf);
}

void app_main(void) {
    // UART for fingerprint sensor
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Init OLED + Keypad
    oled_init();
    keypad_init();
    oled_show_message("OLED + FP + Keypad");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Verify fingerprint sensor
    oled_show_message("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_show_message("Sensor Error");
        return;
    }
    oled_show_message("Sensor OK");

    while (1) {
        oled_show_message("1:Enr 2:Sea 3:Del");

        char option = '\0';
        while ((option = keypad_get_key()) == '\0') {
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if (option == '1' || option == '3') {
            uint16_t id = get_id_from_keypad();
            if (id > 127) {
                oled_show_message("Invalid ID");
                continue;
            }
            if (option == '1') enroll_fingerprint(id);
            else delete_fingerprint(id);
        } else if (option == '2') {
            search_fingerprint();
        } else {
            oled_show_message("Invalid Opt");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as608.h"
#include "oled.h"
#include "keypad.h"

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

void enroll_fingerprint(uint16_t pageID) {
    uint8_t status;
    oled_show_message("Enroll Start");

    oled_show_message("Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured 1");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        oled_show_message("GenChar1 Fail");
        return;
    }

    oled_show_message("Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    oled_show_message("Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured 2");

    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) {
        oled_show_message("GenChar2 Fail");
        return;
    }

    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) {
        oled_show_message("RegModel Fail");
        return;
    }

    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) {
        oled_show_message("Store Fail");
        return;
    }

    oled_show_message("Enroll OK");
}

void search_fingerprint() {
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    oled_show_message("Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        oled_show_message("GenChar Fail");
        return;
    }

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
    if (status == FINGERPRINT_OK) {
        char line2[20];
        snprintf(line2, sizeof(line2), "ID:%d Sc:%d", pageID, score);
        oled_show_message_2line("Match Found", line2);
    } else if (status == FINGERPRINT_NOTFOUND || status == FINGERPRINT_NOMATCH) {
        oled_show_message("No Match");
    } else {
        oled_show_message("Search Fail");
    }
}

void delete_fingerprint(uint16_t pageID) {
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);
    if (status == FINGERPRINT_OK) {
        oled_show_message("Delete OK");
    } else {
        oled_show_message("Delete Fail");
    }
}

uint16_t get_id_from_keypad() {
    char id_buf[4] = {0};
    int idx = 0;
    oled_show_message("Enter ID (0-127)");

    while (idx < 3) {
        char key = keypad_get_key();
        if (key >= '0' && key <= '9') {
            id_buf[idx++] = key;
            char disp[16];
            snprintf(disp, sizeof(disp), "ID: %s", id_buf);
            oled_show_message(disp);
        } else if (key == '#') {
            break; // Confirm
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return atoi(id_buf);
}

void app_main(void) {
    // UART config for fingerprint sensor
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Init OLED & Keypad
    oled_init();
    keypad_init();
    oled_show_message("FP+OLED+Keypad");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Verify fingerprint sensor
    oled_show_message("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_show_message("Sensor Error");
        return;
    }
    oled_show_message("Sensor OK");

    // ---------------- MAIN LOOP ----------------
    while (1) {
        // Regular search mode
        oled_show_message("Searching...");
        search_fingerprint();

        // Check for # to enter menu
        for (int i = 0; i < 30; ++i) { // Scan for # key for 3s
            char key = keypad_get_key();
            if (key == '#') {
                oled_show_message("Menu Mode");
                vTaskDelay(pdMS_TO_TICKS(500));
                goto menu_mode;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;

        // ---------------- MENU MODE ----------------
        menu_mode:
        while (1) {
            oled_show_message("1:Enr 2:Del #:Exit");
            char menu_key = '\0';
            while ((menu_key = keypad_get_key()) == '\0') {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if (menu_key == '#') {
                oled_show_message("Exit Menu");
                break; // Exit menu and go back to search
            } else if (menu_key == '1' || menu_key == '2') {
                uint16_t id = get_id_from_keypad();
                if (id > 127) {
                    oled_show_message("Invalid ID");
                    continue;
                }
                if (menu_key == '1') enroll_fingerprint(id);
                else delete_fingerprint(id);
            } else {
                oled_show_message("Invalid Option");
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}





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

*/

/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as608.h"
#include "oled.h"
#include "keypad.h"
#include "ds1307.h"

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

void enroll_fingerprint(uint16_t pageID) {
    uint8_t status;
    oled_show_message("Enroll Start");

    oled_show_message("Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured 1");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        oled_show_message("GenChar1 Fail");
        return;
    }

    oled_show_message("Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    oled_show_message("Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured 2");

    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) {
        oled_show_message("GenChar2 Fail");
        return;
    }

    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) {
        oled_show_message("RegModel Fail");
        return;
    }

    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) {
        oled_show_message("Store Fail");
        return;
    }

    oled_show_message("Enroll OK");
}

void search_fingerprint() {
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    oled_show_message("Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        oled_show_message("GenChar Fail");
        return;
    }

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
    if (status == FINGERPRINT_OK) {
        char line2[20];
        snprintf(line2, sizeof(line2), "ID:%d Sc:%d", pageID, score);
        oled_show_message_2line("Match Found", line2);
    } else if (status == FINGERPRINT_NOTFOUND || status == FINGERPRINT_NOMATCH) {
        oled_show_message("No Match");
    } else {
        oled_show_message("Search Fail");
    }
}

void delete_fingerprint(uint16_t pageID) {
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);
    if (status == FINGERPRINT_OK) {
        oled_show_message("Delete OK");
    } else {
        oled_show_message("Delete Fail");
    }
}

uint16_t get_id_from_keypad() {
    char id_buf[4] = {0};
    int idx = 0;
    oled_show_message("Enter ID (0-127)");

    while (idx < 3) {
        char key = keypad_get_key();
        if (key >= '0' && key <= '9') {
            id_buf[idx++] = key;
            char disp[16];
            snprintf(disp, sizeof(disp), "ID: %s", id_buf);
            oled_show_message(disp);
        } else if (key == '#') {
            break; // Confirm
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return atoi(id_buf);
}

void app_main(void) {
    // UART config for fingerprint sensor
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Init OLED & Keypad
    oled_init();
    keypad_init();
    oled_show_message("FP+OLED+Keypad");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
     // Show time immediately after welcome
    uint8_t hour, min, sec;
    if (ds1307_get_time(&hour, &min, &sec)) {
        char time_str[20];
        snprintf(time_str, sizeof(time_str), "Time: %02d:%02d:%02d", hour, min, sec);
        oled_show_message(time_str);
    } else {
        oled_show_message("RTC Error");
    }

    vTaskDelay(pdMS_TO_TICKS(2000));  // Show time briefly before proceeding

    // Continue with fingerprint sensor check
    oled_show_message("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_show_message("Sensor Error");
        return;
    }
    oled_show_message("Sensor OK");
    // Verify fingerprint sensor
    oled_show_message("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_show_message("Sensor Error");
        return;
    }
    oled_show_message("Sensor OK");

    // ---------------- MAIN LOOP ----------------
    while (1) {
        // Regular search mode
        oled_show_message("Searching...");
        search_fingerprint();

        // Check for # to enter menu
        for (int i = 0; i < 30; ++i) { // Scan for # key for 3s
            char key = keypad_get_key();
            if (key == '#') {
                oled_show_message("Menu Mode");
                vTaskDelay(pdMS_TO_TICKS(500));
                goto menu_mode;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;

        // ---------------- MENU MODE ----------------
        menu_mode:
        while (1) {
            oled_show_message("1:Enr 2:Del #:Exit");
            char menu_key = '\0';
            while ((menu_key = keypad_get_key()) == '\0') {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if (menu_key == '#') {
                oled_show_message("Exit Menu");
                break; // Exit menu and go back to search
            } else if (menu_key == '1' || menu_key == '2') {
                uint16_t id = get_id_from_keypad();
                if (id > 127) {
                    oled_show_message("Invalid ID");
                    continue;
                }
                if (menu_key == '1') enroll_fingerprint(id);
                else delete_fingerprint(id);
            } else {
                oled_show_message("Invalid Option");
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
*/
/*
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "as608.h"
#include "oled.h"
#include "keypad.h"
#include "ds1307.h"

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

void enroll_fingerprint(uint16_t pageID) {
    uint8_t status;
    oled_show_message("Enroll Start");

    oled_show_message("Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured 1");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        oled_show_message("GenChar1 Fail");
        return;
    }

    oled_show_message("Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    oled_show_message("Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured 2");

    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) {
        oled_show_message("GenChar2 Fail");
        return;
    }

    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) {
        oled_show_message("RegModel Fail");
        return;
    }

    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) {
        oled_show_message("Store Fail");
        return;
    }

    oled_show_message("Enroll OK");
}

void search_fingerprint() {
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    oled_show_message("Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    oled_show_message("Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) {
        oled_show_message("GenChar Fail");
        return;
    }

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
    if (status == FINGERPRINT_OK) {
        char line2[20];
        snprintf(line2, sizeof(line2), "ID:%d Sc:%d", pageID, score);
        oled_show_message_2line("Match Found", line2);
    } else if (status == FINGERPRINT_NOTFOUND || status == FINGERPRINT_NOMATCH) {
        oled_show_message("No Match");
    } else {
        oled_show_message("Search Fail");
    }
}

void delete_fingerprint(uint16_t pageID) {
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);
    if (status == FINGERPRINT_OK) {
        oled_show_message("Delete OK");
    } else {
        oled_show_message("Delete Fail");
    }
}

uint16_t get_id_from_keypad() {
    char id_buf[4] = {0};
    int idx = 0;
    oled_show_message("Enter ID (0-127)");

    while (idx < 3) {
        char key = keypad_get_key();
        if (key >= '0' && key <= '9') {
            id_buf[idx++] = key;
            char disp[16];
            snprintf(disp, sizeof(disp), "ID: %s", id_buf);
            oled_show_message(disp);
        } else if (key == '#') {
            break; // Confirm
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return atoi(id_buf);
}

void app_main(void) {
    // UART config for fingerprint sensor
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Init OLED & Keypad
    oled_init();
    keypad_init();
    oled_show_message("FP+OLED+Keypad");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
     // Show time immediately after welcome
    uint8_t hour, min, sec;
    if (ds1307_get_time(&hour, &min, &sec)) {
        char time_str[20];
        snprintf(time_str, sizeof(time_str), "Time: %02d:%02d:%02d", hour, min, sec);
        oled_show_message(time_str);
    } else {
        oled_show_message("RTC Error");
    }

    vTaskDelay(pdMS_TO_TICKS(2000));  // Show time briefly before proceeding

    // Continue with fingerprint sensor check
    oled_show_message("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_show_message("Sensor Error");
        return;
    }
    oled_show_message("Sensor OK");
    // Verify fingerprint sensor
    oled_show_message("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_show_message("Sensor Error");
        return;
    }
    oled_show_message("Sensor OK");

    // ---------------- MAIN LOOP ----------------
    while (1) {
        // Regular search mode
        oled_show_message("Searching...");
        search_fingerprint();

        // Check for # to enter menu
        for (int i = 0; i < 30; ++i) { // Scan for # key for 3s
            char key = keypad_get_key();
            if (key == '#') {
                oled_show_message("Menu Mode");
                vTaskDelay(pdMS_TO_TICKS(500));
                goto menu_mode;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;

        // ---------------- MENU MODE ----------------
        menu_mode:
        while (1) {
            oled_show_message("1:Enr 2:Del #:Exit");
            char menu_key = '\0';
            while ((menu_key = keypad_get_key()) == '\0') {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if (menu_key == '#') {
                oled_show_message("Exit Menu");
                break; // Exit menu and go back to search
            } else if (menu_key == '1' || menu_key == '2') {
                uint16_t id = get_id_from_keypad();
                if (id > 127) {
                    oled_show_message("Invalid ID");
                    continue;
                }
                if (menu_key == '1') enroll_fingerprint(id);
                else delete_fingerprint(id);
            } else {
                oled_show_message("Invalid Option");
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

*/
/*

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_connect.h"  // Your custom component

void app_main(void) {
    // Initialize NVS (required for Wi-Fi to work)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Connect to Wi-Fi
    wifi_init_sta();

    // Delay and monitor
    for (;;) {
        ESP_LOGI("APP", "Waiting... WiFi should be connected.");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_connect.h"

void app_main(void) {
    // Initialize NVS (used by Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Connect to Wi-Fi
    wifi_init_sta();

    // Log periodically
    while (true) {
        ESP_LOGI("APP", " Wi-Fi connection code is running...");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "wifi_connect.h"

void app_main(void) {
    // Initialize NVS (used by Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Connect to Wi-Fi
    wifi_init_sta();

    // Log periodically
    while (true) {
        ESP_LOGI("APP", " Wi-Fi connection code is running...");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "time.h"
#include "esp_sntp.h"
#include "wifi_connect.h"

static const char* TAG = "MAIN";

void initialize_sntp() {
    ESP_LOGI(TAG, "Initializing SNTP...");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");  // You can change to "time.google.com"
    sntp_init();
}

void obtain_time() {
    initialize_sntp();

    // Wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    while (timeinfo.tm_year < (2020 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for time sync... (%d)", retry);
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (retry == retry_count) {
        ESP_LOGW(TAG, "Time sync failed");
        return;
    }

    // Set time zone to Sri Lanka (UTC+5:30)
    setenv("TZ", "IST-5:30", 1);
    tzset();

    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "🕒 Current Sri Lanka Time: %02d:%02d:%02d",
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void app_main(void) {
    // NVS is needed for Wi-Fi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Connect to Wi-Fi
    wifi_init_sta();

    // Wait until connected
    vTaskDelay(pdMS_TO_TICKS(5000)); // Wait for IP
    obtain_time();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "time.h"
#include "esp_sntp.h"
#include "wifi_connect.h"

static const char* TAG = "MAIN";

void initialize_sntp() {
    ESP_LOGI(TAG, "Initializing SNTP...");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");  // or use "time.google.com"
    sntp_init();
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Connect to Wi-Fi
    wifi_init_sta();

    // Wait for IP address
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Start SNTP
    initialize_sntp();

    // Wait for time sync
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0, max_retries = 10;
    while (timeinfo.tm_year < (2020 - 1900) && ++retry < max_retries) {
        ESP_LOGI(TAG, "Waiting for time sync... (%d)", retry);
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (retry == max_retries) {
        ESP_LOGW(TAG, "Time sync failed.");
        return;
    }

    // Set Sri Lanka timezone
    setenv("TZ", "IST-5:30", 1);
    tzset();

    // 🔁 Print time continuously
    while (1) {
        time(&now);
        localtime_r(&now, &timeinfo);
        ESP_LOGI(TAG, "🕒 Sri Lanka Time: %04d-%02d-%02d %02d:%02d:%02d",
                 timeinfo.tm_year + 1900,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_mday,
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec);
        vTaskDelay(pdMS_TO_TICKS(10000));  // every 10 seconds
    }
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "time.h"

#include "as608.h"
#include "oled.h"
#include "keypad.h"
#include "ds1307.h"
#include "wifi_connect.h"

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128
static const char* TAG = "MAIN";

// === Fingerprint Related Functions (unchanged) ===

void enroll_fingerprint(uint16_t pageID) {
    uint8_t status;
    oled_show_message("Enroll Start");

    oled_show_message("Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_show_message("Captured 1");
    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    oled_show_message("Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) vTaskDelay(pdMS_TO_TICKS(500));

    oled_show_message("Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_show_message("Captured 2");
    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) return;

    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) return;
    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) return;

    oled_show_message("Enroll OK");
}

void search_fingerprint() {
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    oled_show_message("Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_show_message("Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
    if (status == FINGERPRINT_OK) {
        char line2[20];
        snprintf(line2, sizeof(line2), "ID:%d Sc:%d", pageID, score);
        oled_show_message_2line("Match Found", line2);
    } else {
        oled_show_message("No Match");
    }
}

void delete_fingerprint(uint16_t pageID) {
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);
    if (status == FINGERPRINT_OK)
        oled_show_message("Delete OK");
    else
        oled_show_message("Delete Fail");
}

uint16_t get_id_from_keypad() {
    char id_buf[4] = {0};
    int idx = 0;
    oled_show_message("Enter ID (0-127)");

    while (idx < 3) {
        char key = keypad_get_key();
        if (key >= '0' && key <= '9') {
            id_buf[idx++] = key;
            char disp[16];
            snprintf(disp, sizeof(disp), "ID: %s", id_buf);
            oled_show_message(disp);
        } else if (key == '#') break;
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return atoi(id_buf);
}

// === Time from Internet ===
void obtain_sri_lanka_time() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    while (timeinfo.tm_year < (2020 - 1900) && retry++ < 10) {
        time(&now);
        localtime_r(&now, &timeinfo);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    setenv("TZ", "IST-5:30", 1);  // UTC+5:30
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);

    char time_msg[32];
    snprintf(time_msg, sizeof(time_msg), "Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    oled_show_message(time_msg);
    ESP_LOGI(TAG, "🕒 Internet Time: %s", time_msg);
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void app_main(void) {
    // UART for fingerprint
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Init peripherals
    oled_init();
    keypad_init();

    oled_show_message("FP+OLED+Keypad");
    vTaskDelay(pdMS_TO_TICKS(2000));

    // NVS for Wi-Fi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Wi-Fi Connect
    wifi_init_sta();
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Show IP
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char ip_msg[32];
        snprintf(ip_msg, sizeof(ip_msg), "IP:%d.%d.%d.%d", IP2STR(&ip_info.ip));
        oled_show_message_2line("WiFi Connected", ip_msg);
    } else {
        oled_show_message("WiFi Conn Failed");
    }
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Show SNTP time
    obtain_sri_lanka_time();

    // Show RTC time
    uint8_t hour, min, sec;
    if (ds1307_get_time(&hour, &min, &sec)) {
        char time_str[20];
        snprintf(time_str, sizeof(time_str), "RTC: %02d:%02d:%02d", hour, min, sec);
        oled_show_message(time_str);
    } else {
        oled_show_message("RTC Error");
    }
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Verify fingerprint sensor
    oled_show_message("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_show_message("Sensor Error");
        return;
    }
    oled_show_message("Sensor OK");

    // ---------------- Main Loop ----------------
    while (1) {
        oled_show_message("Searching...");
        search_fingerprint();

        for (int i = 0; i < 30; ++i) {
            char key = keypad_get_key();
            if (key == '#') {
                oled_show_message("Menu Mode");
                vTaskDelay(pdMS_TO_TICKS(500));
                goto menu_mode;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;

        // ----------- Menu Mode -----------
        menu_mode:
        while (1) {
            oled_show_message("1:Enr 2:Del #:Exit");
            char menu_key = '\0';
            while ((menu_key = keypad_get_key()) == '\0') {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if (menu_key == '#') {
                oled_show_message("Exit Menu");
                break;
            } else if (menu_key == '1' || menu_key == '2') {
                uint16_t id = get_id_from_keypad();
                if (id > 127) {
                    oled_show_message("Invalid ID");
                    continue;
                }
                if (menu_key == '1') enroll_fingerprint(id);
                else delete_fingerprint(id);
            } else {
                oled_show_message("Invalid Option");
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "time.h"

#include "as608.h"
#include "oled.h"
#include "keypad.h"
#include "ds1307.h"
#include "wifi_connect.h"

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

static const char* TAG = "MAIN";

// Time update task
void time_display_task(void *pvParameters) {
    char time_str[32];
    time_t now;
    struct tm timeinfo;

    while (1) {
        time(&now);
        localtime_r(&now, &timeinfo);
        snprintf(time_str, sizeof(time_str), "Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        oled_set_line(0, time_str); // Assume oled_set_line updates only line 1
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void enroll_fingerprint(uint16_t pageID) {
    uint8_t status;
    oled_set_line(1, "Enroll Start");

    oled_set_line(1, "Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_set_line(1, "Captured 1");
    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    oled_set_line(1, "Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) vTaskDelay(pdMS_TO_TICKS(500));

    oled_set_line(1, "Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_set_line(1, "Captured 2");
    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) return;

    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) return;
    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) return;

    oled_set_line(1, "Enroll OK");
}

void search_fingerprint() {
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    oled_set_line(1, "Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_set_line(1, "Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
    if (status == FINGERPRINT_OK) {
        char line2[20];
        snprintf(line2, sizeof(line2), "ID:%d Sc:%d", pageID, score);
        oled_show_message_2line("Match Found", line2);
    } else {
         oled_show_message("No Match");
    }
}

void delete_fingerprint(uint16_t pageID) {
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);
    oled_set_line(1, (status == FINGERPRINT_OK) ? "Delete OK" : "Delete Fail");
}

uint16_t get_id_from_keypad() {
    char id_buf[4] = {0};
    int idx = 0;
    oled_set_line(1, "Enter ID (0-127)");

    while (idx < 3) {
        char key = keypad_get_key();
        if (key >= '0' && key <= '9') {
            id_buf[idx++] = key;
            char disp[16];
            snprintf(disp, sizeof(disp), "ID: %s", id_buf);
            oled_set_line(1, disp);
        } else if (key == '#') break;
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return atoi(id_buf);
}

void obtain_sri_lanka_time() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    while (timeinfo.tm_year < (2020 - 1900) && retry++ < 10) {
        time(&now);
        localtime_r(&now, &timeinfo);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    setenv("TZ", "IST-5:30", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "\xf0\x9f\x95\x92 Internet Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void app_main(void) {
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    oled_init();
    keypad_init();

    oled_set_line(1, "FP+OLED+Keypad");
    vTaskDelay(pdMS_TO_TICKS(2000));

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    wifi_init_sta();
    vTaskDelay(pdMS_TO_TICKS(5000));

    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char ip_msg[32];
        snprintf(ip_msg, sizeof(ip_msg), "IP:%d.%d.%d.%d", IP2STR(&ip_info.ip));
        oled_show_message_2line("WiFi Connected", ip_msg);
    } else {
        oled_set_line(1, "WiFi Conn Failed");
    }
    vTaskDelay(pdMS_TO_TICKS(2000));

    obtain_sri_lanka_time();
    xTaskCreate(time_display_task, "time_display_task", 2048, NULL, 5, NULL);

    oled_set_line(1, "Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_set_line(1, "Sensor Error");
        return;
    }
    oled_set_line(1, "Sensor OK");

    while (1) {
        oled_set_line(1, "Searching...");
        search_fingerprint();

        for (int i = 0; i < 30; ++i) {
            char key = keypad_get_key();
            if (key == '#') {
                oled_set_line(1, "Menu Mode");
                vTaskDelay(pdMS_TO_TICKS(500));
                goto menu_mode;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;

        menu_mode:
        while (1) {
            oled_set_line(1, "1:Enr 2:Del #:Exit");
            char menu_key = '\0';
            while ((menu_key = keypad_get_key()) == '\0') {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if (menu_key == '#') {
                oled_set_line(1, "Exit Menu");
                break;
            } else if (menu_key == '1' || menu_key == '2') {
                uint16_t id = get_id_from_keypad();
                if (id > 127) {
                    oled_set_line(1, "Invalid ID");
                    continue;
                }
                if (menu_key == '1') enroll_fingerprint(id);
                else delete_fingerprint(id);
            } else {
                oled_set_line(1, "Invalid Option");
            }

            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

*/
/*

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "time.h"

#include "as608.h"
#include "oled.h"
#include "keypad.h"
#include "ds1307.h"
#include "wifi_connect.h"

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

static const char* TAG = "MAIN";

// ----------------- Time Task -----------------
void time_display_task(void *pvParameters) {
    char time_str[32];
    time_t now;
    struct tm timeinfo;
    while (1) {
        time(&now);
        localtime_r(&now, &timeinfo);
        snprintf(time_str, sizeof(time_str), "Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        oled_set_line(0, time_str);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ----------------- Fingerprint Tasks -----------------
void enroll_fingerprint(uint16_t pageID) {
    uint8_t status;
    oled_set_line(1, "Enroll Start");

    oled_set_line(1, "Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_set_line(1, "Captured 1");
    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    oled_set_line(1, "Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) vTaskDelay(pdMS_TO_TICKS(500));

    oled_set_line(1, "Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_set_line(1, "Captured 2");
    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) return;

    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) return;
    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) return;

    oled_set_line(1, "Enroll OK");
    vTaskDelay(pdMS_TO_TICKS(1500));
    oled_clear_line(1);
}

void search_fingerprint() {
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    oled_set_line(1, "Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_set_line(1, "Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
    if (status == FINGERPRINT_OK) {
        char info[20];
        snprintf(info, sizeof(info), "ID:%d Sc:%d", pageID, score);
        oled_set_line(1, "Match Found");
        oled_set_line(2, info);
        vTaskDelay(pdMS_TO_TICKS(2000));
        oled_clear_line(1);
        oled_clear_line(2);
    } else {
        oled_set_line(1, "No Match");
        vTaskDelay(pdMS_TO_TICKS(1500));
        oled_clear_line(1);
    }
}

void delete_fingerprint(uint16_t pageID) {
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);
    oled_set_line(1, (status == FINGERPRINT_OK) ? "Delete OK" : "Delete Fail");
    vTaskDelay(pdMS_TO_TICKS(1500));
    oled_clear_line(1);
}

uint16_t get_id_from_keypad() {
    char id_buf[4] = {0};
    int idx = 0;
    oled_set_line(1, "Enter ID (0-127)");

    while (idx < 3) {
        char key = keypad_get_key();
        if (key >= '0' && key <= '9') {
            id_buf[idx++] = key;
            char disp[16];
            snprintf(disp, sizeof(disp), "ID: %s", id_buf);
            oled_set_line(1, disp);
        } else if (key == '#') break;
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    return atoi(id_buf);
}

// ----------------- SNTP -----------------
void obtain_sri_lanka_time() {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    while (timeinfo.tm_year < (2020 - 1900) && retry++ < 10) {
        time(&now);
        localtime_r(&now, &timeinfo);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    setenv("TZ", "IST-5:30", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "🕒 Internet Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}


// ----------------- MAIN -----------------
void app_main(void) {
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    oled_init();
    keypad_init();
    oled_set_line(1, "FP+OLED+Keypad");
    vTaskDelay(pdMS_TO_TICKS(2000));
    oled_clear_line(1);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    wifi_init_sta();
    vTaskDelay(pdMS_TO_TICKS(5000));

    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char ip_msg[32];
        snprintf(ip_msg, sizeof(ip_msg), "IP:%d.%d.%d.%d", IP2STR(&ip_info.ip));
        oled_set_line(1, "WiFi Connected");
        oled_set_line(2, ip_msg);
        vTaskDelay(pdMS_TO_TICKS(2000));
        oled_clear_line(1);
        oled_clear_line(2);
    } else {
        oled_set_line(1, "WiFi Conn Failed");
    }

    obtain_sri_lanka_time();
    xTaskCreate(time_display_task, "time_display_task", 2048, NULL, 5, NULL);

    oled_set_line(1, "Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_set_line(1, "Sensor Error");
        return;
    }
    oled_set_line(1, "Sensor OK");
    vTaskDelay(pdMS_TO_TICKS(1500));
    oled_clear_line(1);

    while (1) {
        oled_set_line(1, "Searching...");
        search_fingerprint();

        for (int i = 0; i < 30; ++i) {
            char key = keypad_get_key();
            if (key == '#') {
                oled_set_line(1, "Menu Mode");
                vTaskDelay(pdMS_TO_TICKS(500));
                goto menu_mode;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;

        menu_mode:
        while (1) {
            oled_set_line(1, "1:Enr 2:Del #:Exit");
            char menu_key = '\0';
            while ((menu_key = keypad_get_key()) == '\0') {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if (menu_key == '#') {
                oled_set_line(1, "Exit Menu");
                vTaskDelay(pdMS_TO_TICKS(1000));
                oled_clear_line(1);
                break;
            } else if (menu_key == '1' || menu_key == '2') {
                uint16_t id = get_id_from_keypad();
                if (id > 127) {
                    oled_set_line(1, "Invalid ID");
                    continue;
                }
                if (menu_key == '1') enroll_fingerprint(id);
                else delete_fingerprint(id);
            } else {
                oled_set_line(1, "Invalid Option");
                vTaskDelay(pdMS_TO_TICKS(1000));
                oled_clear_line(1);
            }
        }
    }
}
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "time.h"

#include "as608.h"
#include "oled.h"
#include "keypad.h"
#include "ds1307.h"
#include "wifi_connect.h"

#define UART_NUM UART_NUM_2
#define UART_TXD 17
#define UART_RXD 16
#define BUF_SIZE 128

static const char* TAG = "MAIN";

// ----------------- Time Task -----------------
void time_display_task(void *pvParameters) {
    char time_str[32];
    time_t now;
    struct tm timeinfo;
    while (1) {
        time(&now);
        localtime_r(&now, &timeinfo);
        snprintf(time_str, sizeof(time_str), "Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        oled_set_line0(time_str);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ----------------- Fingerprint Tasks -----------------
void enroll_fingerprint(uint16_t pageID) {
    uint8_t status;
    oled_set_line3("Enroll Start");
    oled_clear_line4();
    oled_set_line3("Place 1st finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_clear_line3();
    oled_set_line3("Captured 1");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    oled_clear_line3();
    oled_set_line3("Remove Finger");
    vTaskDelay(pdMS_TO_TICKS(1500));
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_NOFINGER) vTaskDelay(pdMS_TO_TICKS(500));

    oled_set_line3("Place 2nd finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_clear_line3();
    oled_set_line3("Captured 2");

    if ((status = AS608_GenChar(UART_NUM, 2)) != FINGERPRINT_OK) return;
    if ((status = AS608_RegModel(UART_NUM)) != FINGERPRINT_OK) return;
    if ((status = AS608_Store(UART_NUM, 1, pageID)) != FINGERPRINT_OK) return;

    oled_clear_line3();
    oled_set_line3("Enroll OK");
    vTaskDelay(pdMS_TO_TICKS(1500));
    oled_clear_line3();
}

void search_fingerprint() {
    uint8_t status;
    uint16_t pageID = 0, score = 0;

    oled_set_line3("Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) vTaskDelay(pdMS_TO_TICKS(500));
    oled_clear_line3();
    oled_set_line3("Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
    if (status == FINGERPRINT_OK) {
        char info[20];
        snprintf(info, sizeof(info), "ID:%d Sc:%d", pageID, score);
        oled_clear_line3();
        oled_set_line3("Match Found");
        oled_set_line4(info);
        vTaskDelay(pdMS_TO_TICKS(2000));
        oled_clear_line3();
        oled_clear_line4();
    } else {
        oled_set_line3("No Match");
        vTaskDelay(pdMS_TO_TICKS(1500));
        oled_clear_line3();
    }
}

void delete_fingerprint(uint16_t pageID) {
    uint8_t status = AS608_Delete(UART_NUM, pageID, 1);
    oled_set_line3((status == FINGERPRINT_OK) ? "Delete OK" : "Delete Fail");
    vTaskDelay(pdMS_TO_TICKS(1500));
    oled_clear_line3();
}

uint16_t get_id_from_keypad() {
    char id_buf[4] = {0};
    int idx = 0;
    oled_set_line3("Enter ID (0-127)");
    oled_clear_line4();

    while (idx < 3) {
        char key = keypad_get_key();
        if (key >= '0' && key <= '9') {
            id_buf[idx++] = key;
            char disp[16];
            snprintf(disp, sizeof(disp), "ID: %s", id_buf);
            oled_set_line4(disp);
        } else if (key == '#') break;
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    return atoi(id_buf);
}

// ----------------- SNTP -----------------
void obtain_sri_lanka_time() {
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    while (timeinfo.tm_year < (2020 - 1900) && retry++ < 10) {
        time(&now);
        localtime_r(&now, &timeinfo);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    setenv("TZ", "IST-5:30", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    ESP_LOGI(TAG, "🕒 Internet Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

// ----------------- Main -----------------
void app_main(void) {
    uart_config_t uart_config = {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    oled_init();
    keypad_init();
    oled_set_line3("FP+OLED+Keypad");
    vTaskDelay(pdMS_TO_TICKS(2000));
    oled_clear_line3();

    // Init Wi-Fi
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    vTaskDelay(pdMS_TO_TICKS(5000));

    // IP Info Display
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char ip_msg[32];
        snprintf(ip_msg, sizeof(ip_msg), "IP:%d.%d.%d.%d", IP2STR(&ip_info.ip));
        oled_set_line3("WiFi Connected");
        oled_set_line4(ip_msg);
        vTaskDelay(pdMS_TO_TICKS(2000));
        oled_clear_line3();
        oled_clear_line4();
    } else {
        oled_set_line3("WiFi Conn Failed");
        vTaskDelay(pdMS_TO_TICKS(2000));
        oled_clear_line3();
    }

    obtain_sri_lanka_time();
    xTaskCreate(time_display_task, "time_display_task", 2048, NULL, 5, NULL);

    oled_set_line3("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_set_line3("Sensor Error");
        return;
    }
    oled_set_line3("Sensor OK");
    vTaskDelay(pdMS_TO_TICKS(1500));
    oled_clear_line3();

    // --------- Main Loop ---------
    while (1) {
        oled_set_line3("Searching...");
        search_fingerprint();

        for (int i = 0; i < 30; ++i) {
            char key = keypad_get_key();
            if (key == '#') {
                oled_set_line3("Menu Mode");
                vTaskDelay(pdMS_TO_TICKS(500));
                oled_clear_line3();
                goto menu_mode;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        continue;

    menu_mode:
        while (1) {
            oled_set_line3("1:Enr 2:Del #:Exit");
            oled_clear_line4();
            char menu_key = '\0';
            while ((menu_key = keypad_get_key()) == '\0') {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if (menu_key == '#') {
                oled_set_line3("Exit Menu");
                vTaskDelay(pdMS_TO_TICKS(1000));
                oled_clear_line3();
                break;
            } else if (menu_key == '1' || menu_key == '2') {
                uint16_t id = get_id_from_keypad();
                if (id > 127) {
                    oled_set_line3("Invalid ID");
                    vTaskDelay(pdMS_TO_TICKS(1500));
                    oled_clear_line3();
                    continue;
                }
                if (menu_key == '1') enroll_fingerprint(id);
                else delete_fingerprint(id);
            } else {
                oled_set_line3("Invalid Option");
                vTaskDelay(pdMS_TO_TICKS(1000));
                oled_clear_line3();
            }
        }
    }
}
