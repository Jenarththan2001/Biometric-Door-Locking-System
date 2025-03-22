#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"

// UART and GPIO configuration
#define UART_NUM UART_NUM_2   // Use UART2 for communication
#define TX_PIN 17             // GPIO17 connected to AS608 RX (transmit pin)
#define RX_PIN 16             // GPIO16 connected to AS608 TX (receive pin)
#define BUF_SIZE 128          // UART RX buffer size

static const char *TAG = "AS608_CHECK";  // Log tag for ESP_LOG

// Handshake command to initiate communication with AS608 fingerprint sensor
// This is a fixed packet format as defined in the AS608 datasheet
uint8_t handshake_cmd[] = {
    0xEF, 0x01,              // Packet header
    0xFF, 0xFF, 0xFF, 0xFF,  // Default address (4 bytes)
    0x01,                    // Packet identifier (Command packet)
    0x00, 0x03,              // Packet length = 3 bytes
    0x01, 0x00, 0x05         // Command: Handshake (0x01), checksum = 0x0005
};

// Function to initialize UART2 for AS608 communication
void init_uart() {
    uart_config_t uart_config = {
        .baud_rate = 57600,                    // Baud rate required by AS608
        .data_bits = UART_DATA_8_BITS,         // 8 data bits
        .parity = UART_PARITY_DISABLE,         // No parity
        .stop_bits = UART_STOP_BITS_1,         // 1 stop bit
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE  // No flow control
    };

    uart_param_config(UART_NUM, &uart_config);  // Apply UART config
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);  // Set TX/RX pins
    uart_driver_install(UART_NUM, BUF_SIZE, 0, 0, NULL, 0);  // Install UART driver with RX buffer only
}

// Function to send a command to AS608 fingerprint sensor
void send_command(uint8_t *cmd, size_t len) {
    uart_write_bytes(UART_NUM, (const char *)cmd, len);  // Transmit command via UART
}

// Function to check if fingerprint sensor is connected and responding
void check_fingerprint_sensor() {
    uint8_t rx_data[BUF_SIZE];  // Buffer to store received response

    send_command(handshake_cmd, sizeof(handshake_cmd));  // Send handshake command
    ESP_LOGI(TAG, "Sent Handshake Command");

    vTaskDelay(pdMS_TO_TICKS(500));  // Wait for 500ms to receive response

    // Read response from UART with 500ms timeout
    int len = uart_read_bytes(UART_NUM, rx_data, BUF_SIZE, pdMS_TO_TICKS(500));

    if (len > 0) {
        ESP_LOGI(TAG, "Fingerprint Sensor Detected! Response:");
        // Print the received bytes in hex format
        for (int i = 0; i < len; i++) {
            printf("0x%02X ", rx_data[i]);
        }
        printf("\n");
    } else {
        ESP_LOGE(TAG, "No response! Check wiring.");  // Error log if no response received
    }
}

// FreeRTOS task to continuously check fingerprint sensor every 5 seconds
void fingerprint_task(void *pvParameter) {
    while (1) {
        check_fingerprint_sensor();  // Attempt handshake
        vTaskDelay(pdMS_TO_TICKS(5000));  // Wait for 5 seconds before next check
    }
}

// Entry point of the application
void app_main() {
    init_uart();  // Initialize UART communication
    xTaskCreate(fingerprint_task, "fingerprint_task", 4096, NULL, 5, NULL);  // Start fingerprint task
}
