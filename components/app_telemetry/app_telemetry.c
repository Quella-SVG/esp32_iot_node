#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "app_config.h"
#include "sht30.h"
#include "onenet.h"
#include "app_telemetry.h"

static const char *TAG = "telemetry";

/* ---------- 数据源注册表：加传感器 = 加一行，业务代码不动 ---------- */
typedef struct {
    const char *stream_name;             /* 数据流名（与平台一致） */
    esp_err_t (*read)(float *out);       /* 读函数 */
} telemetry_source_t;

static float s_temp, s_humi;

static esp_err_t read_temp(float *out)
{
    esp_err_t r = sht30_read(&s_temp, &s_humi);
    if (r == ESP_OK) *out = s_temp;
    return r;
}

static esp_err_t read_humi(float *out)
{
    esp_err_t r = sht30_read(&s_temp, &s_humi);
    if (r == ESP_OK) *out = s_humi;
    return r;
}

static const telemetry_source_t g_sources[] = {
    { CFG_DS_TEMP, read_temp },
    { CFG_DS_HUMI, read_humi },
    /* 以后加传感器（如光照 BH1750）：{ "Lux", bh1750_read } */
};
#define SOURCE_NUM (sizeof(g_sources) / sizeof(g_sources[0]))

static uint32_t s_period_ms = CFG_TELEMETRY_PERIOD_MS;

static void telemetry_task(void *arg)
{
    /* 业务层负责初始化它用到的传感器 */
    if (sht30_init(CFG_SHT30_SDA, CFG_SHT30_SCL, CFG_SHT30_ADDR) != ESP_OK) {
        ESP_LOGE(TAG, "SHT30 初始化失败，检查接线");
    }
    vTaskDelay(pdMS_TO_TICKS(100));   /* 初始化后稍等，避免首读时序竞争 */

    for (;;) {
        for (int i = 0; i < SOURCE_NUM; i++) {
            float v = 0;
            if (g_sources[i].read(&v) == ESP_OK) {
                if (onenet_report(g_sources[i].stream_name, v) != ESP_OK) {
                    ESP_LOGW(TAG, "%s 上报失败（MQTT 未连接）", g_sources[i].stream_name);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(s_period_ms));
    }
}

esp_err_t app_telemetry_start(void)
{
    xTaskCreate(telemetry_task, "telemetry", 4096, NULL, 5, NULL);
    return ESP_OK;
}

void app_telemetry_on_command(const char *cmd_json)
{
    ESP_LOGI(TAG, "执行命令: %s", cmd_json);

    /* 示例命令：{"cmd":"set_period","value":10000} 修改上报周期 */
    if (strstr(cmd_json, "set_period") && strstr(cmd_json, "value")) {
        const char *p = strstr(cmd_json, "value");
        s_period_ms = (uint32_t)atoi(p + 6);
        ESP_LOGI(TAG, "上报周期改为 %lu ms", (unsigned long)s_period_ms);
    }
}
