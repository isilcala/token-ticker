#include "time_service.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "esp_log.h"
#include "esp_netif_sntp.h"

static const char *TAG = "time_service";

static time_service_state_t s_state;

static const char *time_service_map_timezone(const char *timezone)
{
    if (timezone == NULL || timezone[0] == '\0')
    {
        return "UTC0";
    }

    if (strcmp(timezone, "Asia/Shanghai") == 0)
    {
        return "CST-8";
    }

    return timezone;
}

static void time_service_apply_timezone(const char *timezone)
{
    const char *mapped = time_service_map_timezone(timezone);

    setenv("TZ", mapped, 1);
    tzset();
}

void time_service_init(bool ntp_enabled, uint16_t ntp_sync_hours, const char *timezone)
{
    memset(&s_state, 0, sizeof(s_state));
    s_state.ntp_enabled = ntp_enabled;
    s_state.ntp_sync_hours = ntp_sync_hours;
    if (timezone != NULL)
    {
        strncpy(s_state.timezone, timezone, sizeof(s_state.timezone) - 1);
    }
    time_service_apply_timezone(s_state.timezone);
}

void time_service_mark_time_valid(int64_t epoch_seconds, int64_t monotonic_seconds)
{
    s_state.last_ntp_sync_epoch = epoch_seconds;
    s_state.last_ntp_sync_monotonic_epoch = monotonic_seconds;
    s_state.time_valid = true;
}

bool time_service_should_sync_ntp(int64_t now_epoch_seconds)
{
    int64_t interval_seconds;

    if (!s_state.ntp_enabled)
    {
        return false;
    }

    if (s_state.last_ntp_sync_monotonic_epoch <= 0)
    {
        return true;
    }

    interval_seconds = (int64_t)s_state.ntp_sync_hours * 3600;
    return (now_epoch_seconds - s_state.last_ntp_sync_monotonic_epoch) >= interval_seconds;
}

bool time_service_sync_ntp(uint32_t timeout_ms, int64_t monotonic_seconds, int64_t *epoch_seconds)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_err_t error;
    time_t now;

    if (!s_state.ntp_enabled)
    {
        return false;
    }

    time_service_apply_timezone(s_state.timezone);
    config.wait_for_sync = true;

    esp_netif_sntp_deinit();
    error = esp_netif_sntp_init(&config);
    if (error != ESP_OK)
    {
        ESP_LOGW(TAG, "esp_netif_sntp_init failed err=%s", esp_err_to_name(error));
        return false;
    }

    error = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(timeout_ms));
    if (error != ESP_OK)
    {
        ESP_LOGW(TAG, "sntp sync wait failed err=%s", esp_err_to_name(error));
        esp_netif_sntp_deinit();
        return false;
    }

    time(&now);
    esp_netif_sntp_deinit();

    if (now <= 0)
    {
        return false;
    }

    if (epoch_seconds != NULL)
    {
        *epoch_seconds = (int64_t)now;
    }

    time_service_mark_time_valid((int64_t)now, monotonic_seconds);
    ESP_LOGI(TAG, "ntp sync ok epoch=%lld", (long long)now);
    return true;
}

const time_service_state_t *time_service_get_state(void)
{
    return &s_state;
}