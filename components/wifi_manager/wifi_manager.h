#pragma once
#include "esp_err.h"

/* WiFi 连接状态回调：connected=true 表示已拿到 IP */
typedef void (*wifi_evt_cb_t)(bool connected, void *arg);

/* 启动 WiFi 连接，断线自动重连 */
esp_err_t wifi_mgr_start(const char *ssid, const char *pass,
                         wifi_evt_cb_t cb, void *arg);
