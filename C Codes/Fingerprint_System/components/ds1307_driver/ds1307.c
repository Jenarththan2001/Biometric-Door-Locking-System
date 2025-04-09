#include "ds1307.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define DS1307_ADDR 0x68
#define I2C_MASTER_SCL_IO   18
#define I2C_MASTER_SDA_IO   19
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_FREQ_HZ  100000
#define TAG "DS1307"

static uint8_t bcd_to_dec(uint8_t val) {
    return ((val / 16 * 10) + (val % 16));
}

static uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10 * 16) + (val % 10));
}

void ds1307_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

bool ds1307_get_time(uint8_t* hour, uint8_t* min, uint8_t* sec) {
    uint8_t reg = 0x00;
    uint8_t data[3];

    if (i2c_master_write_read_device(I2C_MASTER_NUM, DS1307_ADDR, &reg, 1, data, 3, pdMS_TO_TICKS(100)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from DS1307");
        return false;
    }

    *sec = bcd_to_dec(data[0] & 0x7F);
    *min = bcd_to_dec(data[1]);
    *hour = bcd_to_dec(data[2] & 0x3F);
    return true;
}
bool ds1307_set_time(uint8_t hour, uint8_t min, uint8_t sec) {
    uint8_t time_data[4];
    time_data[0] = 0x00;                  // Start at register 0x00
    time_data[1] = dec_to_bcd(sec);      // Seconds
    time_data[2] = dec_to_bcd(min);      // Minutes
    time_data[3] = dec_to_bcd(hour);     // Hours

    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, DS1307_ADDR, time_data, 4, pdMS_TO_TICKS(100));
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Time set to %02d:%02d:%02d", hour, min, sec);
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to set time");
        return false;
    }
}


