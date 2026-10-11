#include <time.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "net_time.h"

static const char *TAG = "net_time";

esp_err_t net_time_sync(uint32_t timeout_ms)
{
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "ntp.aliyun.com");
    esp_sntp_init();

    /* 真实时间 > 2020-09 才算同步成功（token 的 et 依赖真实时钟） */
    time_t now = 0;
    uint32_t waited = 0;
    while (now < 1600000000 && waited < timeout_ms) {
        vTaskDelay(pdMS_TO_TICKS(500));
        time(&now);
        waited += 500;
    }
    if (now < 1600000000) {
        ESP_LOGW(TAG, "时间同步超时");
        return ESP_ERR_TIMEOUT;
    }
    setenv("TZ", "CST-8", 1);   /* 东八区，仅影响本地时间显示 */
    tzset();
    ESP_LOGI(TAG, "时间同步完成");
    return ESP_OK;
}
