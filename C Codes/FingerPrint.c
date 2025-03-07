#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define UART_NUM UART_NUM_2  // Use UART2
#define TX_PIN 17            // GPIO17 -> AS608 RX
#define RX_PIN 16            // GPIO16 -> AS608 TX
#define BUF_SIZE 128

static const char *TAG = "AS608_CHECK";

// Handshake command for AS608 (Fixed Packet Format)
uint8_t handshake_cmd[] = {
    0xEF, 0x01,        // Header
    0xFF, 0xFF, 0xFF, 0xFF,  // Address (Default)
    0x01,              // Packet Identifier (Command Packet)
    0x00, 0x03,        // Packet Length (3 bytes)
    0x01, 0x00, 0x05   // Command: Handshake + Checksum
};

// Function to initialize UART2
void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = 57600,  // Default baud rate for AS608
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE, 0, 0, NULL, 0);
}

// Function to send a command to AS608
void send_command(uint8_t *cmd, size_t len) {
    uart_write_bytes(UART_NUM, (const char *)cmd, len);
}

// Function to receive and check response
void check_fingerprint_sensor() {
    uint8_t rx_data[BUF_SIZE];
    
    // Send handshake command
    send_command(handshake_cmd, sizeof(handshake_cmd));
    ESP_LOGI(TAG, "Sent Handshake Command");

    vTaskDelay(pdMS_TO_TICKS(500));  // Wait for response

    int len = uart_read_bytes(UART_NUM, rx_data, BUF_SIZE, pdMS_TO_TICKS(500));

    if (len > 0) {
        ESP_LOGI(TAG, "Fingerprint Sensor Detected! Response:");
        for (int i = 0; i < len; i++) {
            printf("0x%02X ", rx_data[i]);  // Print received bytes
        }
        printf("\n");
    } else {
        ESP_LOGE(TAG, "No response! Check wiring.");
    }
}

// Main task
void fingerprint_task(void *pvParameter) {
    while (1) {
        check_fingerprint_sensor();
        vTaskDelay(pdMS_TO_TICKS(5000));  // Check every 5 seconds
    }
}

void app_main() {
    init_uart();  // Initialize UART
    xTaskCreate(fingerprint_task, "fingerprint_task", 4096, NULL, 5, NULL);
}
