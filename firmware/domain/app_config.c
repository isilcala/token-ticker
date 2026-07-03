#include "app_config.h"

#include <string.h>

static bool string_has_value(const char *value)
{
    return value != NULL && value[0] != '\0';
}

static uint16_t normalized_sleep_schedule_minutes(uint16_t minutes)
{
    return minutes == (24U * 60U) ? 0U : minutes;
}

void app_config_init(app_config_t *config)
{
    if (config == NULL)
    {
        return;
    }

    memset(config, 0, sizeof(*config));
    config->version = 1;
    config->source = CONFIG_SOURCE_NONE;
    config->device.ntp_enabled = true;
    config->device.ntp_sync_hours = 24;
    strncpy(config->device.timezone, "Asia/Shanghai", sizeof(config->device.timezone) - 1);
    config->device.sleep_schedule.enabled = false;
    config->device.sleep_schedule.wake_minutes = APP_CONFIG_TIME_UNSET;
    config->device.sleep_schedule.sleep_minutes = APP_CONFIG_TIME_UNSET;
    config->device.sleep_schedule.manual_wake_minutes = 5;
    config->wifi.enabled = false;
}

const provider_config_t *app_config_find_active_provider(const app_config_t *config)
{
    size_t index;

    if (config == NULL || !string_has_value(config->display.active_provider_id))
    {
        return NULL;
    }

    for (index = 0; index < config->provider_count; ++index)
    {
        const provider_config_t *provider = &config->providers[index];

        if (provider->enabled && strcmp(provider->id, config->display.active_provider_id) == 0)
        {
            return provider;
        }
    }

    return NULL;
}

bool app_config_validate(const app_config_t *config)
{
    size_t index;

    if (config == NULL)
    {
        return false;
    }

    if (config->version == 0 || config->provider_count > APP_CONFIG_MAX_PROVIDERS)
    {
        return false;
    }

    if (config->wifi.enabled && !string_has_value(config->wifi.ssid))
    {
        return false;
    }

    if (config->device.sleep_schedule.enabled)
    {
        const sleep_schedule_config_t *schedule = &config->device.sleep_schedule;

        if (schedule->wake_minutes == APP_CONFIG_TIME_UNSET ||
            schedule->sleep_minutes == APP_CONFIG_TIME_UNSET)
        {
            return false;
        }

        if (schedule->wake_minutes >= (24U * 60U) ||
            schedule->sleep_minutes > (24U * 60U))
        {
            return false;
        }

        if (normalized_sleep_schedule_minutes(schedule->wake_minutes) ==
            normalized_sleep_schedule_minutes(schedule->sleep_minutes))
        {
            return false;
        }
    }

    if (config->device.sleep_schedule.manual_wake_minutes > (24U * 60U))
    {
        return false;
    }

    for (index = 0; index < config->provider_count; ++index)
    {
        const provider_config_t *provider = &config->providers[index];
        size_t compare_index;

        if (!string_has_value(provider->id))
        {
            return false;
        }

        if (provider->enabled)
        {
            if (provider->provider_type == PROVIDER_TYPE_UNKNOWN || !string_has_value(provider->api_key))
            {
                return false;
            }
        }

        for (compare_index = index + 1; compare_index < config->provider_count; ++compare_index)
        {
            if (strcmp(provider->id, config->providers[compare_index].id) == 0)
            {
                return false;
            }
        }
    }

    if (string_has_value(config->display.active_provider_id) && app_config_find_active_provider(config) == NULL)
    {
        return false;
    }

    return true;
}