#ifndef TB_HTTP_H
#define TB_HTTP_H

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048

#ifdef __cplusplus
extern "C" {
#endif

void tb_http_init(void);
esp_err_t tb_http_send_json(const char *json_payload);

#ifdef __cplusplus
}
#endif

#endif /* TB_HTTPS_H */