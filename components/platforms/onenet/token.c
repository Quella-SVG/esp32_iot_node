#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mbedtls/md.h"
#include "mbedtls/base64.h"
#include "token.h"

/* URL 编码：等价于 Python 的 quote(safe='')，token 必须编码 */
static void url_encode(const char *src, char *dst, size_t dst_len)
{
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 4 < dst_len; i++) {
        unsigned char c = src[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
            dst[j++] = c;
        else {
            snprintf(dst + j, 4, "%%%02X", c);
            j += 3;
        }
    }
    dst[j] = '\0';
}

void one_net_token(const char *product_id, const char *device_name,
                   const char *access_key_b64, char *out, size_t out_len)
{
    /* ① accessKey base64 解码 */
    unsigned char key[64];
    size_t key_len = 0;
    mbedtls_base64_decode(key, sizeof(key), &key_len,
                          (const unsigned char *)access_key_b64,
                          strlen(access_key_b64));

    /* ② 拼签名原文  et\nmethod\nres\nversion */
    char res[128], et[32], org[256];
    snprintf(res, sizeof(res), "products/%s/devices/%s", product_id, device_name);
    snprintf(et, sizeof(et), "%lld", (long long)time(NULL) + 63072000);  /* 2 年有效 */
    snprintf(org, sizeof(org), "%s\n%s\n%s\n2018-10-31", et, "md5", res);

    /* ③ HMAC-MD5 签名 → base64 */
    unsigned char sign_raw[16], sign_b64[64];
    size_t b64len = 0;
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_MD5);
    mbedtls_md_hmac(md, key, key_len, (const unsigned char *)org, strlen(org), sign_raw);
    mbedtls_base64_encode(sign_b64, sizeof(sign_b64), &b64len,
                          sign_raw, sizeof(sign_raw));

    /* ④ res、sign URL 编码后拼 token */
    char res_enc[160], sign_enc[96];
    url_encode(res, res_enc, sizeof(res_enc));
    url_encode((char *)sign_b64, sign_enc, sizeof(sign_enc));
    snprintf(out, out_len, "version=2018-10-31&res=%s&et=%s&method=md5&sign=%s",
             res_enc, et, sign_enc);
}
