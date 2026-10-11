#pragma once

/* ============================================================
 * 全局配置：改参数只改这一个文件
 * WiFi 与 OneNET 身份等敏感信息在 app_config_private.h
 * （该文件不入 git，见 app_config_private.h.example 模板）
 * ============================================================ */

/* ---------- 私有配置：WiFi / OneNET 账号 ---------- */
#include "app_config_private.h"

/* ---------- OneNET 平台 ---------- */
#define CFG_ONENET_URI          "mqtt://183.230.40.96:1883"

/* ---------- 数据流名（必须与平台建的完全一致） ---------- */
#define CFG_DS_TEMP             "Temp"
#define CFG_DS_HUMI             "RH"

/* ---------- SHT30 I2C 引脚 ---------- */
#define CFG_SHT30_SDA           21
#define CFG_SHT30_SCL           8
#define CFG_SHT30_ADDR          0x44

/* ---------- 业务采集周期 ---------- */
#define CFG_TELEMETRY_PERIOD_MS 1000
