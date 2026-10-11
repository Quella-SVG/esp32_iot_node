#pragma once
#include "esp_err.h"

/* OneNET 连接配置 */
typedef struct {
    const char *uri;          /* MQTT 服务器地址，如 "mqtt://183.230.40.96:1883" */
    const char *product_id;   /* 产品 ID */
    const char *device_name;  /* 设备名 */
    const char *access_key;   /* 产品 accessKey（base64 串） */
} onenet_cfg_t;

/* 平台命令回调：cmd_json 为收到的命令内容（只读，用完即失效） */
typedef void (*onenet_cmd_cb_t)(const char *cmd_json);

/* 启动 OneNET MQTT 连接（内部用 accessKey 动态算 token，开机重算不过期） */
esp_err_t onenet_start(const onenet_cfg_t *cfg, onenet_cmd_cb_t cmd_cb);

/* 上报一个数据点；未连接时返回 ESP_ERR_INVALID_STATE */
esp_err_t onenet_report(const char *datastream, float value);
