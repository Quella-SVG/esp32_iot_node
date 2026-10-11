#pragma once
#include "esp_err.h"

/* 初始化 I2C 并配置 SHT30（引脚/地址由调用方传入，驱动不依赖全局配置） */
esp_err_t sht30_init(int sda, int scl, uint8_t addr);

/* 读取温湿度：*temp 单位摄氏度，*humi 相对湿度百分比 */
esp_err_t sht30_read(float *temp, float *humi);
