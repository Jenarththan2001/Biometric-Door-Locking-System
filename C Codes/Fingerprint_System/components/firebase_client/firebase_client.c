#include <stdio.h>
#include <string.h>
#include <time.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_timer.h"

extern const uint8_t firebase_cert_pem_start[] asm("_binary_firebase_cert_pem_start");
extern const uint8_t firebase_cert_pem_end[]   asm("_binary_firebase_cert_pem_end");

#define FIREBASE_URL   "https://fingerprintscanner-1b2d6-default-rtdb.firebaseio.com/user_input.json"
static const char *TAG = "FIREBASE";

void send_firebase_log(int user_id, int access_granted) {
    char json[256];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

    snprintf(json, sizeof(json),
        "{\"UserID\": %d, \"AccessGranted\": %d, \"Timestamp\": \"%s\"}",
        user_id, access_granted, timestamp);

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
        ESP_LOGI(TAG, "Firebase record sent: %s", json);
    } else {
        ESP_LOGE(TAG, "Failed to send Firebase record: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
