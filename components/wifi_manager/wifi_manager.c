#include <string.h>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "wifi_manager.h"

static const char *TAG = "wifi_mgr";

static wifi_evt_cb_t s_cb;
static void *s_arg;
static esp_timer_handle_t s_reconnect_timer = NULL;

static void do_reconnect(void *arg)
{
    esp_wifi_connect();
}

static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t id, void *event_data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        /* 开机射频未稳定时立刻连接容易 auth 超时，延迟 500ms 再连 */
        esp_timer_start_once(s_reconnect_timer, 500 * 1000);
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_cb) s_cb(false, s_arg);
        esp_timer_start_once(s_reconnect_timer, 1000 * 1000);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&e->ip_info.ip));
        if (s_cb) s_cb(true, s_arg);
    }
}

esp_err_t wifi_mgr_start(const char *ssid, const char *pass,
                         wifi_evt_cb_t cb, void *arg)
{
    s_cb = cb;
    s_arg = arg;

    esp_timer_create_args_t targs = {
        .callback = do_reconnect,
        .name = "wifi_reconnect",
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&targs, &s_reconnect_timer), TAG, "create timer failed");

    ESP_RETURN_ON_ERROR(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                   wifi_event_handler, NULL), TAG, "reg wifi evt failed");
    ESP_RETURN_ON_ERROR(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                                   wifi_event_handler, NULL), TAG, "reg ip evt failed");

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&init_cfg), TAG, "wifi init failed");

    wifi_config_t w = { 0 };
    strlcpy((char *)w.sta.ssid, ssid, sizeof(w.sta.ssid));
    strlcpy((char *)w.sta.password, pass, sizeof(w.sta.password));
    w.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "set mode failed");
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &w), TAG, "set config failed");
    return esp_wifi_start();
}