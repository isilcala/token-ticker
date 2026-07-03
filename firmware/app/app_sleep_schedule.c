#include "app_sleep_schedule.h"

#include <string.h>
#include <time.h>

enum
{
    SECONDS_PER_DAY = 24 * 60 * 60,
    MINUTES_PER_DAY = 24 * 60,
};

static uint16_t normalized_minutes(uint16_t minutes)
{
    return minutes == MINUTES_PER_DAY ? 0U : minutes;
}

static bool rtc_time_to_epoch(const rtc_time_t *rtc_time, int64_t *epoch_seconds)
{
    struct tm time_info;
    time_t local_epoch;

    if (rtc_time == NULL || epoch_seconds == NULL || !rtc_time->valid)
    {
        return false;
    }

    memset(&time_info, 0, sizeof(time_info));
    time_info.tm_year = (int)rtc_time->year - 1900;
    time_info.tm_mon = (int)rtc_time->month - 1;
    time_info.tm_mday = (int)rtc_time->day;
    time_info.tm_hour = (int)rtc_time->hour;
    time_info.tm_min = (int)rtc_time->minute;
    time_info.tm_sec = (int)rtc_time->second;
    time_info.tm_isdst = -1;

    local_epoch = mktime(&time_info);
    if (local_epoch < 0)
    {
        return false;
    }

    *epoch_seconds = (int64_t)local_epoch;
    return true;
}

static bool epoch_to_rtc_time(int64_t epoch_seconds, rtc_time_t *rtc_time)
{
    time_t raw_time = (time_t)epoch_seconds;
    struct tm time_info;

    if (rtc_time == NULL || epoch_seconds <= 0)
    {
        return false;
    }

    memset(&time_info, 0, sizeof(time_info));
    if (localtime_r(&raw_time, &time_info) == NULL)
    {
        return false;
    }

    rtc_time->year = (uint16_t)(time_info.tm_year + 1900);
    rtc_time->month = (uint8_t)(time_info.tm_mon + 1);
    rtc_time->day = (uint8_t)time_info.tm_mday;
    rtc_time->hour = (uint8_t)time_info.tm_hour;
    rtc_time->minute = (uint8_t)time_info.tm_min;
    rtc_time->second = (uint8_t)time_info.tm_sec;
    rtc_time->valid = true;
    return true;
}

static int64_t next_occurrence_epoch(const rtc_time_t *rtc_time, int64_t now_epoch_seconds, uint16_t raw_minutes)
{
    const int64_t seconds_since_midnight = ((int64_t)rtc_time->hour * 3600) +
                                           ((int64_t)rtc_time->minute * 60) +
                                           (int64_t)rtc_time->second;
    const int64_t day_start_epoch = now_epoch_seconds - seconds_since_midnight;
    int64_t candidate_epoch = day_start_epoch + ((int64_t)raw_minutes * 60);

    if (candidate_epoch <= now_epoch_seconds)
    {
        candidate_epoch += SECONDS_PER_DAY;
    }

    return candidate_epoch;
}

bool app_sleep_schedule_is_enabled(const app_config_t *config)
{
    return config != NULL && config->device.sleep_schedule.enabled;
}

bool app_sleep_schedule_is_active_now(const app_config_t *config,
                                      const rtc_time_t *rtc_time,
                                      bool *active)
{
    uint16_t wake_minutes;
    uint16_t sleep_minutes;
    uint16_t now_minutes;

    if (config == NULL || rtc_time == NULL || active == NULL || !rtc_time->valid)
    {
        return false;
    }

    if (!app_sleep_schedule_is_enabled(config))
    {
        *active = true;
        return true;
    }

    wake_minutes = normalized_minutes(config->device.sleep_schedule.wake_minutes);
    sleep_minutes = normalized_minutes(config->device.sleep_schedule.sleep_minutes);
    now_minutes = (uint16_t)(((uint16_t)rtc_time->hour * 60U) + (uint16_t)rtc_time->minute);

    if (wake_minutes < sleep_minutes)
    {
        *active = now_minutes >= wake_minutes && now_minutes < sleep_minutes;
        return true;
    }

    *active = now_minutes >= wake_minutes || now_minutes < sleep_minutes;
    return true;
}

bool app_sleep_schedule_seconds_until_transition(const app_config_t *config,
                                                 const rtc_time_t *rtc_time,
                                                 uint32_t *seconds_until_transition,
                                                 bool *next_is_sleep)
{
    bool active;
    int64_t now_epoch_seconds;
    int64_t transition_epoch;
    uint16_t transition_minutes;

    if (config == NULL || rtc_time == NULL || seconds_until_transition == NULL)
    {
        return false;
    }

    if (!app_sleep_schedule_is_enabled(config))
    {
        return false;
    }

    if (!app_sleep_schedule_is_active_now(config, rtc_time, &active) ||
        !rtc_time_to_epoch(rtc_time, &now_epoch_seconds))
    {
        return false;
    }

    transition_minutes = active
                             ? config->device.sleep_schedule.sleep_minutes
                             : config->device.sleep_schedule.wake_minutes;
    transition_epoch = next_occurrence_epoch(rtc_time, now_epoch_seconds, transition_minutes);

    if (transition_epoch <= now_epoch_seconds)
    {
        return false;
    }

    *seconds_until_transition = (uint32_t)(transition_epoch - now_epoch_seconds);
    if (next_is_sleep != NULL)
    {
        *next_is_sleep = active;
    }

    return true;
}

bool app_sleep_schedule_next_transition(const app_config_t *config,
                                        const rtc_time_t *rtc_time,
                                        rtc_time_t *next_transition_time,
                                        bool *next_is_sleep)
{
    uint32_t seconds_until_transition;
    int64_t now_epoch_seconds;

    if (config == NULL || rtc_time == NULL || next_transition_time == NULL)
    {
        return false;
    }

    if (!app_sleep_schedule_seconds_until_transition(config,
                                                     rtc_time,
                                                     &seconds_until_transition,
                                                     next_is_sleep) ||
        !rtc_time_to_epoch(rtc_time, &now_epoch_seconds))
    {
        return false;
    }

    return epoch_to_rtc_time(now_epoch_seconds + (int64_t)seconds_until_transition,
                             next_transition_time);
}