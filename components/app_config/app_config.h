#pragma once

/* ============================================================
 * 全局配置：以后改参数只改这一个文件
 * ============================================================ */

/* ---------- WiFi ---------- */
#define CFG_WIFI_SSID           "***REMOVED***"
#define CFG_WIFI_PASS           "***REMOVED***"

/* ---------- OneNET 平台 ---------- */
#define CFG_ONENET_URI          "mqtt://183.230.40.96:1883"
#define CFG_ONENET_PRODUCT_ID   "***REMOVED***"
#define CFG_ONENET_DEVICE_NAME  "SHT30"
#define CFG_ONENET_ACCESS_KEY   "***REMOVED***"

/* ---------- 数据流名（必须与平台建的完全一致） ---------- */
#define CFG_DS_TEMP             "Temp"
#define CFG_DS_HUMI             "RH"

/* ---------- SHT30 I2C 引脚 ---------- */
#define CFG_SHT30_SDA           21
#define CFG_SHT30_SCL           8
#define CFG_SHT30_ADDR          0x44

/* ---------- 业务采集周期 ---------- */
#define CFG_TELEMETRY_PERIOD_MS 1000
