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
