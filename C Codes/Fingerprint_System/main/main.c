
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "soc/gpio_num.h"
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

#define BIT_MENU_MODE (1 << 0)

#define GREEN_LED GPIO_NUM_18
#define RED_LED   GPIO_NUM_19
#define BUZZER_GPIO GPIO_NUM_2
#define BUZZER_FREQ 1000  // 2kHz for buzzer
#define BUZZER_DUTY 4000  // Duty out of 8191 (~50%)

static const char* TAG = "MAIN";
static EventGroupHandle_t event_group;



// Buzzer PWM Setup
void init_buzzer_pwm() {
    ledc_timer_config_t buzzer_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = BUZZER_FREQ,
        .speed_mode = LEDC_HIGH_SPEED_MODE,  // FIXED
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&buzzer_timer);

    ledc_channel_config_t buzzer_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_HIGH_SPEED_MODE,  // FIXED
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0
    };
    ledc_channel_config(&buzzer_channel);
}

void buzzer_beep(int freq, int duration_ms) {
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, BUZZER_DUTY);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

/*
void buzzer_on() {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, BUZZER_DUTY);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

void buzzer_off() {
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}
*/

void oled_update_wifi_status() {
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_ip_info_t ip_info;
    char wifi_status[16];

    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK && ip_info.ip.addr != 0) {
        snprintf(wifi_status, sizeof(wifi_status), "    WiFi: ON");
    } else {
        snprintf(wifi_status, sizeof(wifi_status), "   WiFi: OFF");
    }

    oled_set_line7(wifi_status);
}

void time_display_task(void *pvParameters) {
    char time_str[32];
    time_t now;
    struct tm timeinfo;
    while (1) {
        time(&now);
        localtime_r(&now, &timeinfo);
        snprintf(time_str, sizeof(time_str), " Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        oled_set_line1("________________");
        oled_set_line6("----------------");
        oled_update_wifi_status();
        oled_set_line0(time_str);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void keypad_admin_task(void *pvParams) {
    while (1) {
        char key = keypad_get_key();
        if (key == '#') {
            xEventGroupSetBits(event_group, BIT_MENU_MODE);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

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

    oled_set_line3("  Place Finger");
    while (AS608_GetImage(UART_NUM) != FINGERPRINT_OK) {
        if (xEventGroupGetBits(event_group) & BIT_MENU_MODE) return;
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    oled_clear_line3();
    oled_set_line3("Captured");

    if ((status = AS608_GenChar(UART_NUM, 1)) != FINGERPRINT_OK) return;

    status = AS608_Search(UART_NUM, 1, 0, 1000, &pageID, &score);
if (status == FINGERPRINT_OK) {
        char info[20];
        snprintf(info, sizeof(info), "ID:%d Sc:%d", pageID, score);
        oled_clear_line3();
        oled_set_line3("Access Granted");
        oled_set_line4(info);

        gpio_set_level(GREEN_LED, 1);
        buzzer_beep(1500, 1000);  // High-pitch beep for access granted
        gpio_set_level(GREEN_LED, 0);


        oled_clear_line3();
        oled_clear_line4();
    } else {
        oled_set_line3("Access Denied");
        gpio_set_level(RED_LED, 1);
        buzzer_beep(400, 1000);  // Low-pitch beep for access denied
        gpio_set_level(RED_LED, 0);
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
    oled_clear_line5();

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
    ESP_LOGI(TAG, "Internet Time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

void app_main(void) {
	
    init_buzzer_pwm();  // <-- Add this BEFORE buzzer_on/off usage

	gpio_config_t led_conf = {
    .pin_bit_mask = (1ULL << GREEN_LED) | (1ULL << RED_LED),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};
gpio_config(&led_conf);
	
	
	
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
    oled_set_line2("   Welcome to ");
    oled_set_line3("  Finger Print ");
    oled_set_line4("     System   ");
    vTaskDelay(pdMS_TO_TICKS(2500));
    oled_clear_line2();
    oled_clear_line3();
    oled_clear_line4();

    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_sta();
    vTaskDelay(pdMS_TO_TICKS(5000));

    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        char ip_msg[32];
        snprintf(ip_msg, sizeof(ip_msg), "IP:%d.%d.%d.%d", IP2STR(&ip_info.ip));
        oled_set_line3("WiFi Connected");
        oled_set_line4(ip_msg);
        vTaskDelay(pdMS_TO_TICKS(1000));
        oled_clear_line3();
        oled_clear_line4();
    } else {
        oled_set_line3("WiFi Conn Failed");
        vTaskDelay(pdMS_TO_TICKS(1000));
        oled_clear_line3();
    }

    obtain_sri_lanka_time();
    xTaskCreate(time_display_task, "time_display_task", 2048, NULL, 5, NULL);

    event_group = xEventGroupCreate();
    xTaskCreate(keypad_admin_task, "keypad_admin_task", 2048, NULL, 5, NULL);

    oled_set_line3("Checking Sensor");
    if (!AS608_VerifyPassword(UART_NUM)) {
        oled_set_line3("AS608 Error");
        return;
    }
    oled_set_line3("AS608 OK");
    vTaskDelay(pdMS_TO_TICKS(1500));
    oled_clear_line3();

    while (1) {
        if (xEventGroupWaitBits(event_group, BIT_MENU_MODE, pdTRUE, pdFALSE, 0)) {
            goto menu_mode;
        }

        oled_set_line3("Searching...");
        search_fingerprint();

        if (xEventGroupWaitBits(event_group, BIT_MENU_MODE, pdTRUE, pdFALSE, 0)) {
            goto menu_mode;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
        continue;

    menu_mode:
        oled_clear_line3();
        oled_set_line3("Menu Mode");
        vTaskDelay(pdMS_TO_TICKS(500));
        oled_clear_line3();

        while (1) {
            oled_set_line3("1: Enroll");
            oled_set_line4("2: Delete");
            oled_set_line5("#: Exit");

            char menu_key = '\0';
            while ((menu_key = keypad_get_key()) == '\0') {
                vTaskDelay(pdMS_TO_TICKS(100));
            }

            if (menu_key == '#') {
                oled_clear_line3();
                oled_clear_line4();
                oled_clear_line5();
                oled_set_line3("Exit Menu");
                vTaskDelay(pdMS_TO_TICKS(1000));
                oled_clear_line3();
                xEventGroupClearBits(event_group, BIT_MENU_MODE);
                break;
            } else if (menu_key == '1' || menu_key == '2') {
                uint16_t id = get_id_from_keypad();
                if (id > 127) {
                    oled_clear_line3();
                    oled_clear_line4();
                    oled_clear_line5();
                    oled_set_line3("Invalid ID");
                    vTaskDelay(pdMS_TO_TICKS(1500));
                    oled_clear_line3();
                    continue;
                }
                if (menu_key == '1') enroll_fingerprint(id);
                else delete_fingerprint(id);
            } else {
                oled_clear_line3();
                oled_clear_line4();
                oled_clear_line5();
                oled_set_line3("Invalid Option");
                vTaskDelay(pdMS_TO_TICKS(1000));
                oled_clear_line3();
            }
        }
    }
}
