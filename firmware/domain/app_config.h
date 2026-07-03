#ifndef TOKEN_TICKER_APP_CONFIG_H
#define TOKEN_TICKER_APP_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "provider_snapshot.h"

#define APP_CONFIG_MAX_PROVIDERS 4
#define APP_CONFIG_ID_LEN 32
#define APP_CONFIG_SECRET_LEN 128
#define APP_CONFIG_TZ_LEN 48
#define APP_CONFIG_WIFI_SSID_LEN 33
#define APP_CONFIG_WIFI_PASSWORD_LEN 65
#define APP_CONFIG_TIME_UNSET UINT16_MAX

typedef enum
{
    CONFIG_SOURCE_NONE = 0,
    CONFIG_SOURCE_SD_CARD,
    CONFIG_SOURCE_NVS,
    CONFIG_SOURCE_SERIAL,
} config_source_t;

typedef struct
{
    bool enabled;
    uint16_t wake_minutes;
    uint16_t sleep_minutes;
    uint16_t manual_wake_minutes;
} sleep_schedule_config_t;

typedef struct
{
    bool ntp_enabled;
    uint16_t ntp_sync_hours;
    char timezone[APP_CONFIG_TZ_LEN];
    sleep_schedule_config_t sleep_schedule;
} device_config_t;

typedef struct
{
    bool enabled;
    char ssid[APP_CONFIG_WIFI_SSID_LEN];
    char password[APP_CONFIG_WIFI_PASSWORD_LEN];
} app_wifi_config_t;

typedef struct
{
    bool weather_enabled;
    char active_provider_id[APP_CONFIG_ID_LEN];
} display_config_t;

typedef struct
{
    bool enabled;
} weather_config_t;

typedef struct
{
    char id[APP_CONFIG_ID_LEN];
    provider_type_t provider_type;
    bool enabled;
    char region[APP_CONFIG_ID_LEN];
    char api_key[APP_CONFIG_SECRET_LEN];
} provider_config_t;

typedef struct
{
    uint32_t version;
    config_source_t source;
    device_config_t device;
    app_wifi_config_t wifi;
    display_config_t display;
    weather_config_t weather;
    size_t provider_count;
    provider_config_t providers[APP_CONFIG_MAX_PROVIDERS];
} app_config_t;

void app_config_init(app_config_t *config);
bool app_config_validate(const app_config_t *config);
const provider_config_t *app_config_find_active_provider(const app_config_t *config);

#endif