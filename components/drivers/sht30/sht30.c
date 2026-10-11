#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "sht30.h"

static const char *TAG = "sht30";
static i2c_port_t s_port = I2C_NUM_0;
static uint8_t s_addr = 0x44;

/* SHT30 CRC-8：多项式 0x31，初始 0xFF */
static uint8_t sht30_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
    }
    return crc;
}

esp_err_t sht30_init(int sda, int scl, uint8_t addr)
{
    s_addr = addr;

    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    ESP_RETURN_ON_ERROR(i2c_param_config(s_port, &cfg), TAG, "i2c_param_config failed");
    return i2c_driver_install(s_port, I2C_MODE_MASTER, 0, 0, 0);
}

esp_err_t sht30_read(float *temp, float *humi)
{
    uint8_t cmd[2] = { 0x2C, 0x06 };   /* 高重复性测量，无时钟拉伸 */
    ESP_RETURN_ON_ERROR(i2c_master_write_to_device(s_port, s_addr, cmd, 2,
                                                   pdMS_TO_TICKS(50)), TAG, "write failed");
    vTaskDelay(pdMS_TO_TICKS(20));     /* 测量约 15ms，等它测完 */

    uint8_t buf[6];                    /* 温度2字节+CRC + 湿度2字节+CRC */
    ESP_RETURN_ON_ERROR(i2c_master_read_from_device(s_port, s_addr, buf, 6,
                                                    pdMS_TO_TICKS(50)), TAG, "read failed");

    if (sht30_crc8(buf, 2) != buf[2] || sht30_crc8(buf + 3, 2) != buf[5])
        return ESP_ERR_INVALID_CRC;

    uint16_t t_raw = (buf[0] << 8) | buf[1];
    uint16_t h_raw = (buf[3] << 8) | buf[4];
    *temp = -45.0f + 175.0f * t_raw / 65535.0f;   /* SHT30 官方换算公式 */
    *humi = 100.0f * h_raw / 65535.0f;
    return ESP_OK;
}
