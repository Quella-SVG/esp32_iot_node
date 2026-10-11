#pragma once
#include "esp_err.h"

/* SNTP 同步系统时间，timeout_ms 内没同步成功返回超时错误 */
esp_err_t net_time_sync(uint32_t timeout_ms);
