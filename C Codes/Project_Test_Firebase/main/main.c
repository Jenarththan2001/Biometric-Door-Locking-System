#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_timer.h"
#include "esp_sntp.h"
#include <time.h>

extern const uint8_t firebase_cert_pem_start[] asm("_binary_firebase_cert_pem_start");
extern const uint8_t firebase_cert_pem_end[]   asm("_binary_firebase_cert_pem_end");

#define WIFI_SSID      "Nathiskar"
#define WIFI_PASS      "nathis123123"
#define FIREBASE_URL   "https://fingerprintscanner-1b2d6-default-rtdb.firebaseio.com/user_input.json"

#define UART_NUM       UART_NUM_0
#define UART_BUF_SIZE  1024

static const char *TAG = "FIREBASE_UART_INPUT";
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

// --- WiFi Event Handler ---
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        ESP_LOGI(TAG, "WiFi Connected");
    }
}

void wifi_connect() {
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi...");
}

// --- Firebase Upload Task ---
void firebase_send_task(void *param) {
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "WiFi Connected. Sending 2 dummy records...");

    struct {
        int user_id;
        int access_granted;
    } records[2] = {
        {101, 1},  // Record 1: User 101, Access granted
        {102, 0}   // Record 2: User 102, Access denied
    };

    for (int i = 0; i < 2; i++) {
        char json[256];
 time_t now;
time(&now);
struct tm timeinfo;
localtime_r(&now, &timeinfo);

char timestamp[32];
strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

snprintf(json, sizeof(json),
         "{\"UserID\": %d, \"AccessGranted\": %d, \"Timestamp\": \"%s\"}",
         records[i].user_id,
         records[i].access_granted,
         timestamp);


        esp_http_client_config_t config = {
            .url = FIREBASE_URL,
            .method = HTTP_METHOD_POST,
            .cert_pem = (const char *)firebase_cert_pem_start,
            .skip_cert_common_name_check = true,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, json, strlen(json));

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Record %d sent: %s", i + 1, json);
        } else {
            ESP_LOGE(TAG, "Failed to send record %d: %s", i + 1, esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);
        vTaskDelay(pdMS_TO_TICKS(1000));  // Optional delay between records
    }

    vTaskDelete(NULL);  // End the task
}

void initialize_sntp(void) {
    ESP_LOGI(TAG, "Initializing SNTP...");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org"); // or "time.google.com"
    sntp_init();
}

void wait_for_time_sync() {
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;

    while (timeinfo.tm_year < (2023 - 1900) && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for SNTP time sync... (%d)", retry);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}


// --- Main Entry ---
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_connect();
    initialize_sntp();
    wait_for_time_sync();


    // UART initialization (already enabled by default for UART0, just setting up buffer)
    uart_driver_install(UART_NUM, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
    ESP_LOGI(TAG, "UART0 ready. Type message and hit ENTER to send to Firebase.");

    xTaskCreate(firebase_send_task, "firebase_send_task", 8192, NULL, 5, NULL);
}
