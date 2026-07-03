#include "ui_home_view_model.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *WEEKDAY_NAMES[] = {
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
};

static bool ui_home_current_epoch_seconds(const ui_boot_model_t *boot_model, int64_t *epoch_seconds);

static const char *config_result_text(config_store_load_result_t result)
{
    switch (result)
    {
    case CONFIG_STORE_LOAD_FROM_SD:
        return "sd";
    case CONFIG_STORE_LOAD_FROM_NVS:
        return "nvs";
    case CONFIG_STORE_LOAD_SD_OVERRIDES_NVS:
        return "sd_overrides_nvs";
    case CONFIG_STORE_LOAD_UNPROVISIONED:
        return "unprovisioned";
    case CONFIG_STORE_LOAD_ERROR:
    default:
        return "error";
    }
}

static const char *config_result_badge(config_store_load_result_t result)
{
    switch (result)
    {
    case CONFIG_STORE_LOAD_FROM_SD:
        return "SD";
    case CONFIG_STORE_LOAD_FROM_NVS:
        return "NVS";
    case CONFIG_STORE_LOAD_SD_OVERRIDES_NVS:
        return "SD>NVS";
    case CONFIG_STORE_LOAD_UNPROVISIONED:
        return "SETUP";
    case CONFIG_STORE_LOAD_ERROR:
    default:
        return "ERR";
    }
}

static const char *provider_sync_text(provider_sync_state_t sync_state)
{
    switch (sync_state)
    {
    case PROVIDER_SYNC_STATE_OK:
        return "ok";
    case PROVIDER_SYNC_STATE_AUTH_ERROR:
        return "auth_error";
    case PROVIDER_SYNC_STATE_NETWORK_ERROR:
        return "network_error";
    case PROVIDER_SYNC_STATE_PARSE_ERROR:
        return "parse_error";
    case PROVIDER_SYNC_STATE_UNSUPPORTED:
        return "unsupported";
    case PROVIDER_SYNC_STATE_IDLE:
    default:
        return "idle";
    }
}

static void ui_home_format_weekday(const ui_boot_model_t *boot_model, char *target, size_t target_len)
{
    int64_t epoch_seconds;
    time_t raw_time;
    struct tm time_info;

    if (target == NULL || target_len == 0)
    {
        return;
    }

    target[0] = '\0';

    if (!ui_home_current_epoch_seconds(boot_model, &epoch_seconds))
    {
        return;
    }

    raw_time = (time_t)epoch_seconds;
    memset(&time_info, 0, sizeof(time_info));
    if (localtime_r(&raw_time, &time_info) == NULL || time_info.tm_wday < 0 || time_info.tm_wday > 6)
    {
        return;
    }

    snprintf(target, target_len, "%s", WEEKDAY_NAMES[time_info.tm_wday]);
}

static void ui_home_format_hhmm(const rtc_time_t *rtc_time, char *target, size_t target_len)
{
    if (target == NULL || target_len == 0)
    {
        return;
    }

    target[0] = '\0';
    if (rtc_time == NULL || !rtc_time->valid)
    {
        return;
    }

    snprintf(target, target_len, "%02u:%02u", rtc_time->hour, rtc_time->minute);
}

static uint8_t ui_home_wifi_signal_bars(const ui_boot_model_t *boot_model)
{
    if (boot_model == NULL || !boot_model->wifi_has_rssi)
    {
        return 0;
    }

    if (boot_model->wifi_rssi_dbm >= -67)
    {
        return 3;
    }
    if (boot_model->wifi_rssi_dbm >= -77)
    {
        return 2;
    }
    return 1;
}

static void ui_home_format_reset_relative(const quota_window_t *window,
                                          const ui_boot_model_t *boot_model,
                                          char *target,
                                          size_t target_len)
{
    int64_t now_epoch;
    int64_t delta_seconds;
    unsigned minutes;
    unsigned hours;
    unsigned days;

    if (target == NULL || target_len == 0)
    {
        return;
    }

    target[0] = '\0';

    if (window == NULL || window->reset_epoch <= 0 || !ui_home_current_epoch_seconds(boot_model, &now_epoch))
    {
        return;
    }

    delta_seconds = window->reset_epoch - now_epoch;
    if (delta_seconds <= 0)
    {
        snprintf(target, target_len, "R:now");
        return;
    }

    if (delta_seconds < 3600)
    {
        minutes = (unsigned)((delta_seconds + 59) / 60);
        snprintf(target, target_len, "R:%um", minutes);
        return;
    }

    if (delta_seconds < 86400)
    {
        hours = (unsigned)((delta_seconds + 3599) / 3600);
        snprintf(target, target_len, "R:%uh", hours);
        return;
    }

    days = (unsigned)((delta_seconds + 86399) / 86400);
    snprintf(target, target_len, "R:%ud", days);
}

static void ui_home_format_provider_status(provider_sync_state_t sync_state,
                                           char *target,
                                           size_t target_len,
                                           bool *is_ok)
{
    const char *text = "Idle";
    bool ok = false;

    if (target == NULL || target_len == 0)
    {
        return;
    }

    switch (sync_state)
    {
    case PROVIDER_SYNC_STATE_OK:
        text = "Last OK";
        ok = true;
        break;
    case PROVIDER_SYNC_STATE_AUTH_ERROR:
        text = "Auth Err";
        break;
    case PROVIDER_SYNC_STATE_PARSE_ERROR:
        text = "Parse Err";
        break;
    case PROVIDER_SYNC_STATE_UNSUPPORTED:
        text = "Unsupported";
        break;
    case PROVIDER_SYNC_STATE_NETWORK_ERROR:
        text = "Last Fail";
        break;
    case PROVIDER_SYNC_STATE_IDLE:
    default:
        text = "Idle";
        break;
    }

    snprintf(target, target_len, "%s", text);
    if (is_ok != NULL)
    {
        *is_ok = ok;
    }
}

static uint16_t ui_home_used_percent_x100(const quota_window_t *window)
{
    int32_t used_x100;

    if (window == NULL)
    {
        return 0;
    }

    if (window->remaining_percent_x100 > 0)
    {
        used_x100 = 10000 - window->remaining_percent_x100;
        if (used_x100 < 0)
        {
            used_x100 = 0;
        }
        if (used_x100 > 10000)
        {
            used_x100 = 10000;
        }
        return (uint16_t)used_x100;
    }

    if (window->limit <= 0)
    {
        return 0;
    }

    used_x100 = (window->used * 10000) / window->limit;
    if (used_x100 < 0)
    {
        used_x100 = 0;
    }
    if (used_x100 > 10000)
    {
        used_x100 = 10000;
    }
    return (uint16_t)used_x100;
}

static bool ui_home_current_epoch_seconds(const ui_boot_model_t *boot_model, int64_t *epoch_seconds)
{
    struct tm time_info;
    time_t local_epoch;

    if (boot_model == NULL || epoch_seconds == NULL || !boot_model->rtc_time.valid)
    {
        return false;
    }

    memset(&time_info, 0, sizeof(time_info));
    time_info.tm_year = (int)boot_model->rtc_time.year - 1900;
    time_info.tm_mon = (int)boot_model->rtc_time.month - 1;
    time_info.tm_mday = (int)boot_model->rtc_time.day;
    time_info.tm_hour = (int)boot_model->rtc_time.hour;
    time_info.tm_min = (int)boot_model->rtc_time.minute;
    time_info.tm_sec = (int)boot_model->rtc_time.second;
    time_info.tm_isdst = -1;

    local_epoch = mktime(&time_info);
    if (local_epoch < 0)
    {
        return false;
    }

    *epoch_seconds = (int64_t)local_epoch;
    return true;
}

static bool ui_home_time_progress_x100(const ui_boot_model_t *boot_model,
                                       const quota_window_t *window,
                                       uint16_t *progress_x100)
{
    int64_t now_epoch;
    int64_t duration_seconds;
    int64_t elapsed_seconds;
    int64_t progress;

    if (progress_x100 == NULL || window == NULL)
    {
        return false;
    }

    if (window->start_epoch <= 0 || window->reset_epoch <= window->start_epoch)
    {
        return false;
    }

    if (!ui_home_current_epoch_seconds(boot_model, &now_epoch))
    {
        return false;
    }

    duration_seconds = window->reset_epoch - window->start_epoch;
    elapsed_seconds = now_epoch - window->start_epoch;

    if (elapsed_seconds < 0)
    {
        elapsed_seconds = 0;
    }
    if (elapsed_seconds > duration_seconds)
    {
        elapsed_seconds = duration_seconds;
    }

    progress = (elapsed_seconds * 10000) / duration_seconds;
    if (progress < 0)
    {
        progress = 0;
    }
    if (progress > 10000)
    {
        progress = 10000;
    }

    *progress_x100 = (uint16_t)progress;
    return true;
}

static void ui_home_format_reset_compact(const quota_window_t *window,
                                         const ui_boot_model_t *boot_model,
                                         char *target,
                                         size_t target_len)
{
    int64_t now_epoch;
    int64_t delta_seconds;

    if (target == NULL || target_len == 0)
    {
        return;
    }

    target[0] = '\0';

    if (window == NULL || window->reset_epoch <= 0 || !ui_home_current_epoch_seconds(boot_model, &now_epoch))
    {
        return;
    }

    delta_seconds = window->reset_epoch - now_epoch;
    if (delta_seconds <= 0)
    {
        snprintf(target, target_len, "R now");
        return;
    }

    if (delta_seconds < 3600)
    {
        snprintf(target, target_len, "R %um", (unsigned)((delta_seconds + 59) / 60));
        return;
    }

    if (delta_seconds < 86400)
    {
        snprintf(target, target_len, "R %uh", (unsigned)((delta_seconds + 3599) / 3600));
        return;
    }

    snprintf(target, target_len, "R %ud", (unsigned)((delta_seconds + 86399) / 86400));
}

static void ui_home_build_quota_detail(const ui_boot_model_t *boot_model,
                                       const quota_window_t *window,
                                       char *target,
                                       size_t target_len)
{
    char reset_text[UI_HOME_TEXT_LEN];

    if (window == NULL || target == NULL || target_len == 0)
    {
        return;
    }

    ui_home_format_reset_compact(window, boot_model, reset_text, sizeof(reset_text));

    if (strcmp(window->id, "general_5h") == 0)
    {
        snprintf(target,
                 target_len,
                 reset_text[0] != '\0' ? "used %u%%  %s" : "used %u%%",
                 (unsigned)((ui_home_used_percent_x100(window) + 50) / 100),
                 reset_text);
        return;
    }

    if (strcmp(window->id, "general_week") == 0)
    {
        snprintf(target,
                 target_len,
                 reset_text[0] != '\0' ? "used %u%%  %s" : "used %u%%",
                 (unsigned)((ui_home_used_percent_x100(window) + 50) / 100),
                 reset_text);
        return;
    }

    snprintf(target,
             target_len,
             reset_text[0] != '\0' ? "used %ld/%ld  %s" : "used %ld/%ld",
             (long)window->used,
             (long)window->limit,
             reset_text);
}

static void ui_home_build_quota_layout_text(const ui_boot_model_t *boot_model,
                                            const quota_window_t *window,
                                            char *amount_text,
                                            size_t amount_text_len,
                                            char *percent_text,
                                            size_t percent_text_len,
                                            char *detail_left_text,
                                            size_t detail_left_text_len,
                                            char *detail_right_text,
                                            size_t detail_right_text_len)
{
    uint16_t used_x100;

    if (amount_text != NULL && amount_text_len > 0)
    {
        amount_text[0] = '\0';
    }
    if (percent_text != NULL && percent_text_len > 0)
    {
        percent_text[0] = '\0';
    }
    if (detail_left_text != NULL && detail_left_text_len > 0)
    {
        detail_left_text[0] = '\0';
    }
    if (detail_right_text != NULL && detail_right_text_len > 0)
    {
        detail_right_text[0] = '\0';
    }

    if (boot_model == NULL || window == NULL)
    {
        return;
    }

    used_x100 = ui_home_used_percent_x100(window);
    ui_home_format_reset_relative(window, boot_model, detail_right_text, detail_right_text_len);

    if (strcmp(window->id, "video_day") == 0)
    {
        const int32_t remaining = window->limit > window->used ? (window->limit - window->used) : 0;

        snprintf(percent_text,
                 percent_text_len,
                 "%ld/%ld",
                 (long)window->used,
                 (long)window->limit);
        if (remaining > 0)
        {
            snprintf(detail_left_text, detail_left_text_len, "%ld gen left", (long)remaining);
        }
        else
        {
            snprintf(detail_left_text, detail_left_text_len, "No quota");
        }
        return;
    }

    snprintf(percent_text,
             percent_text_len,
             "%u%%",
             (unsigned)((used_x100 + 50) / 100));

    if (ui_home_time_progress_x100(boot_model, window, &used_x100))
    {
        snprintf(detail_left_text,
                 detail_left_text_len,
                 "Time %u%%",
                 (unsigned)((used_x100 + 50) / 100));
    }
}

void ui_home_view_model_build(const ui_boot_model_t *boot_model, ui_home_view_model_t *home_model)
{
    if (boot_model == NULL || home_model == NULL)
    {
        return;
    }

    memset(home_model, 0, sizeof(*home_model));
    home_model->configured = boot_model->configured;
    snprintf(home_model->config_text,
             sizeof(home_model->config_text),
             "%s sd=%s nvs=%s",
             config_result_text(boot_model->config_result),
             boot_model->sd_config_available ? "yes" : "no",
             boot_model->nvs_config_available ? "yes" : "no");
    snprintf(home_model->config_badge_text,
             sizeof(home_model->config_badge_text),
             "%s",
             config_result_badge(boot_model->config_result));

    if (boot_model->rtc_time.valid)
    {
        home_model->has_time = true;
        snprintf(home_model->time_text,
                 sizeof(home_model->time_text),
                 "%02u:%02u",
                 boot_model->rtc_time.hour,
                 boot_model->rtc_time.minute);
        snprintf(home_model->date_text,
                 sizeof(home_model->date_text),
                 "%02u-%02u",
                 boot_model->rtc_time.month,
                 boot_model->rtc_time.day);
        ui_home_format_weekday(boot_model, home_model->weekday_text, sizeof(home_model->weekday_text));
    }

    if (boot_model->environment.valid)
    {
        home_model->has_environment = true;
        snprintf(home_model->environment_text,
                 sizeof(home_model->environment_text),
                 "%ldC %ld%%",
                 (long)((boot_model->environment.temperature_c_x10 >= 0)
                            ? ((boot_model->environment.temperature_c_x10 + 5) / 10)
                            : ((boot_model->environment.temperature_c_x10 - 5) / 10)),
                 (long)((boot_model->environment.humidity_rh_x10 + 5) / 10));
    }

    if (boot_model->power.valid)
    {
        home_model->has_battery = true;
        home_model->has_status_capsule = true;
        snprintf(home_model->battery_text,
                 sizeof(home_model->battery_text),
                 "%ldmV %u%%",
                 (long)boot_model->power.battery_mv,
                 boot_model->power.battery_percent);
        snprintf(home_model->battery_badge_text,
                 sizeof(home_model->battery_badge_text),
                 "%u%%",
                 boot_model->power.battery_percent);
        snprintf(home_model->status_capsule_text,
                 sizeof(home_model->status_capsule_text),
                 "BAT %u%%",
                 boot_model->power.battery_percent);
    }

    if (boot_model->wifi_enabled)
    {
        home_model->has_wifi = true;
        home_model->wifi_connected = boot_model->wifi_connected;
        home_model->wifi_has_signal = boot_model->wifi_has_rssi;
        home_model->wifi_signal_bars = ui_home_wifi_signal_bars(boot_model);
    }

    if (boot_model->has_provider_snapshot)
    {
        const provider_snapshot_t *snapshot = &boot_model->provider_snapshot;
        home_model->has_provider = true;
        snprintf(home_model->provider_name_text,
                 sizeof(home_model->provider_name_text),
                 "%s",
                 snapshot->display_name);
        snprintf(home_model->provider_sync_text,
                 sizeof(home_model->provider_sync_text),
                 "%s%s",
                 provider_sync_text(snapshot->sync_state),
                 snapshot->stale ? "/stale" : "");
        home_model->has_provider_status = true;
        ui_home_format_provider_status(snapshot->sync_state,
                                       home_model->provider_status_text,
                                       sizeof(home_model->provider_status_text),
                                       &home_model->provider_status_ok);

        if (boot_model->has_provider_last_sync_time)
        {
            home_model->has_provider_last_sync = true;
            ui_home_format_hhmm(&boot_model->provider_last_sync_time,
                                home_model->provider_last_sync_text,
                                sizeof(home_model->provider_last_sync_text));
        }

        if (boot_model->has_provider_next_attempt_time)
        {
            home_model->has_provider_next_attempt = true;
            ui_home_format_hhmm(&boot_model->provider_next_attempt_time,
                                home_model->provider_next_attempt_text,
                                sizeof(home_model->provider_next_attempt_text));
        }

        home_model->quota_bar_count = snapshot->window_count < UI_HOME_MAX_QUOTA_BARS
                                          ? snapshot->window_count
                                          : UI_HOME_MAX_QUOTA_BARS;

        if (home_model->quota_bar_count > 0)
        {
            size_t index;

            for (index = 0; index < home_model->quota_bar_count; ++index)
            {
                const quota_window_t *window = &snapshot->windows[index];

                snprintf(home_model->quota_bar_label[index],
                         sizeof(home_model->quota_bar_label[index]),
                         "%s",
                         window->label);
                ui_home_build_quota_detail(boot_model,
                                           window,
                                           home_model->quota_bar_value_text[index],
                                           sizeof(home_model->quota_bar_value_text[index]));
                ui_home_build_quota_layout_text(boot_model,
                                                window,
                                                home_model->quota_bar_amount_text[index],
                                                sizeof(home_model->quota_bar_amount_text[index]),
                                                home_model->quota_bar_percent_text[index],
                                                sizeof(home_model->quota_bar_percent_text[index]),
                                                home_model->quota_bar_detail_left_text[index],
                                                sizeof(home_model->quota_bar_detail_left_text[index]),
                                                home_model->quota_bar_detail_right_text[index],
                                                sizeof(home_model->quota_bar_detail_right_text[index]));
                home_model->quota_bar_used_x100[index] = ui_home_used_percent_x100(window);

                home_model->quota_bar_has_time_marker[index] = false;

                if (strcmp(window->id, "video_day") == 0 && window->limit > 0 && window->limit <= UI_HOME_MAX_QUOTA_SEGMENTS)
                {
                    home_model->quota_bar_segmented[index] = true;
                    home_model->quota_bar_segment_total[index] = (uint8_t)window->limit;
                    home_model->quota_bar_segment_used[index] = (uint8_t)((window->used > window->limit) ? window->limit : window->used);
                }
                else
                {
                    home_model->quota_bar_has_time_marker[index] = ui_home_time_progress_x100(boot_model,
                                                                                              window,
                                                                                              &home_model->quota_bar_time_x100[index]);
                }
            }
        }

        if (home_model->has_status_capsule)
        {
            snprintf(home_model->status_capsule_text,
                     sizeof(home_model->status_capsule_text),
                     "%s",
                     home_model->provider_sync_text);
        }
        else
        {
            home_model->has_status_capsule = true;
            snprintf(home_model->status_capsule_text,
                     sizeof(home_model->status_capsule_text),
                     "%s",
                     home_model->provider_sync_text);
        }
    }
}