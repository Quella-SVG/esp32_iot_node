#pragma once
#include "esp_err.h"

/* 启动业务任务：初始化传感器并周期采集上报 */
esp_err_t app_telemetry_start(void);

/* 平台命令处理（注册给 onenet 的回调，由 main 接线） */
void app_telemetry_on_command(const char *cmd_json);
