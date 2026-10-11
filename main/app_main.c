/**
 * esp32_iot_node 入口：只做装配，不写业务逻辑
 *
 * 启动流程：WiFi 连上 → 同步时间 → 启动 OneNET → 启动业务采集
 */
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_config.h"
#include "wifi_manager.h"
#include "net_time.h"
#include "onenet.h"
#include "app_telemetry.h"

static const char *TAG = "main";

/* OneNET 配置：从 app_config.h 组装，参数都集中在那边改 */
static const onenet_cfg_t g_onenet_cfg = {
    .uri         = CFG_ONENET_URI,
    .product_id  = CFG_ONENET_PRODUCT_ID,
    .device_name = CFG_ONENET_DEVICE_NAME,
    .access_key  = CFG_ONENET_ACCESS_KEY,
};

/* 初始化任务：token 计算（512B 数组 + mbedtls HMAC）栈占用大，
 * 若直接在 WiFi 回调里执行会挤爆事件循环任务(2KB)的栈，
 * 所以单独开任务、给足栈。 */
static void startup_task(void *arg)
{
    ESP_LOGI(TAG, "同步时间...");
    net_time_sync(20000);

    ESP_LOGI(TAG, "启动 OneNET 与业务任务");
    onenet_start(&g_onenet_cfg, app_telemetry_on_command);
    app_telemetry_start();

    vTaskDelete(NULL);
}

/* WiFi 连接状态回调 */
static void on_wifi_event(bool connected, void *arg)
{
    if (connected) {
        ESP_LOGI(TAG, "WiFi 已连接，启动初始化任务");
        xTaskCreate(startup_task, "startup", 6144, NULL, 5, NULL);
    } else {
        ESP_LOGW(TAG, "WiFi 断开，等待自动重连");
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "esp32_iot_node 启动");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    /* 连上 WiFi 后，上面的回调里自动启动上层模块 */
    wifi_mgr_start(CFG_WIFI_SSID, CFG_WIFI_PASS, on_wifi_event, NULL);
}