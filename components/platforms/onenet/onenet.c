#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "token.h"
#include "onenet.h"

static const char *TAG = "onenet";

static esp_mqtt_client_handle_t s_client = NULL;
static onenet_cfg_t s_cfg;
static onenet_cmd_cb_t s_cmd_cb = NULL;
static int s_seq = 0;

/* MQTT 事件处理 */
static void mqtt_event_handler(void *a, esp_event_base_t base,
                               int32_t id, void *data)
{
    esp_mqtt_event_t *ev = data;
    switch (id) {
    case MQTT_EVENT_CONNECTED: {
        char sub[96];
        snprintf(sub, sizeof(sub), "$sys/%s/%s/#", s_cfg.product_id, s_cfg.device_name);
        esp_mqtt_client_subscribe(s_client, sub, 1);
        ESP_LOGI(TAG, "MQTT 已连接，订阅 %s", sub);
        break;
    }
    case MQTT_EVENT_DATA:
        /* 只处理命令下发主题：.../cmd/request/{cmdId} */
        if (strstr(ev->topic, "/cmd/request/")) {
            char *buf = malloc(ev->data_len + 1);
            if (buf) {
                memcpy(buf, ev->data, ev->data_len);
                buf[ev->data_len] = '\0';
                ESP_LOGI(TAG, "收到命令: %s", buf);
                if (s_cmd_cb) s_cmd_cb(buf);

                /* 回执：topic 里 request 换成 response */
                char resp[128];
                strncpy(resp, ev->topic, sizeof(resp) - 1);
                resp[sizeof(resp) - 1] = '\0';
                char *p = strstr(resp, "/request/");
                if (p) memcpy(p + 1, "response", 8);
                esp_mqtt_client_publish(s_client, resp, "{\"data\":\"ok\"}", 0, 1, 0);
                free(buf);
            }
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT 断开，自动重连");
        esp_mqtt_client_reconnect(s_client);
        break;
    default:
        break;
    }
}

esp_err_t onenet_start(const onenet_cfg_t *cfg, onenet_cmd_cb_t cmd_cb)
{
    s_cfg = *cfg;
    s_cmd_cb = cmd_cb;

    /* 动态算 token：开机重算，永不过期 */
    char token[512];
    one_net_token(cfg->product_id, cfg->device_name, cfg->access_key, token, sizeof(token));
    ESP_LOGI(TAG, "token: %s", token);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = cfg->uri,
        .credentials.username = cfg->product_id,
        .credentials.client_id = cfg->device_name,
        .credentials.authentication.password = token,
    };
    s_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    return esp_mqtt_client_start(s_client);
}

esp_err_t onenet_report(const char *datastream, float value)
{
    if (!s_client) return ESP_ERR_INVALID_STATE;

    char topic[96];
    snprintf(topic, sizeof(topic), "$sys/%s/%s/dp/post/json",
             s_cfg.product_id, s_cfg.device_name);

    /* 数据流格式：{"id":序号,"dp":{"数据流名":[{"v":值}]}} */
    cJSON *item = cJSON_CreateObject();
    char vstr[16];
    snprintf(vstr, sizeof(vstr), "%.1f", value);   /* 数据只保留 1 位小数 */
    cJSON_AddRawToObject(item, "v", vstr);
    cJSON *arr = cJSON_CreateArray();
    cJSON_AddItemToArray(arr, item);
    cJSON *dp = cJSON_CreateObject();
    cJSON_AddItemToObject(dp, datastream, arr);
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "id", ++s_seq);
    cJSON_AddItemToObject(root, "dp", dp);

    char *s = cJSON_PrintUnformatted(root);
    int msg_id = esp_mqtt_client_publish(s_client, topic, s, 0, 1, 0);
    ESP_LOGI(TAG, "上报 %s=%.2f -> %s", datastream, value, s);
    free(s);
    cJSON_Delete(root);

    return (msg_id >= 0) ? ESP_OK : ESP_ERR_INVALID_STATE;
}
