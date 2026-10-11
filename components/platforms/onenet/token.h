#pragma once
#include <stddef.h>

/* 用产品 accessKey 计算 MQTT 登录 token（新版 OneNET 密码） */
void one_net_token(const char *product_id, const char *device_name,
                   const char *access_key_b64, char *out, size_t out_len);
