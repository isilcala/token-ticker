#ifndef TOKEN_TICKER_WIFI_PLATFORM_H
#define TOKEN_TICKER_WIFI_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>

#include "app_config.h"

bool wifi_platform_start(const app_config_t *config);
bool wifi_platform_ensure_ready(const app_config_t *config, uint32_t timeout_ms);
bool wifi_platform_wait_ready(uint32_t timeout_ms);
bool wifi_platform_stop(void);
bool wifi_platform_is_ready(void);
bool wifi_platform_get_rssi_dbm(int8_t *rssi_dbm);
const char *wifi_platform_status_text(void);

#endif