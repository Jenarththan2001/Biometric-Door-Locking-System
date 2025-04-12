#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_log.h"

#define BUZZER_GPIO GPIO_NUM_2
#define BUZZER_FREQ 1000  // 2kHz for buzzer
#define BUZZER_DUTY 4000  // Duty out of 8191 (~50%)

static const char *TAG = "BUZZER_PWM";

void app_main(void) {
    // Configure PWM for the buzzer
    ledc_timer_config_t buzzer_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // 2^13 = 8192 steps
        .freq_hz = BUZZER_FREQ,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&buzzer_timer);

    ledc_channel_config_t buzzer_channel = {
        .channel    = LEDC_CHANNEL_0,
        .duty       = 0,
        .gpio_num   = BUZZER_GPIO,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_TIMER_0
    };
    ledc_channel_config(&buzzer_channel);

    while (1) {
        ESP_LOGI(TAG, "Buzzer ON");
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, BUZZER_DUTY);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "Buzzer OFF");
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
