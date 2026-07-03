#include "app_runtime.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_key_input.h"
#include "app_sleep_schedule.h"
#include "provider_registry.h"
#include "time_service.h"
#include "ui_app.h"
#include "wifi_platform.h"

static const char *TAG = "app_runtime";
static const uint32_t APP_RUNTIME_TASK_STACK_SIZE = 8192;
static const uint32_t WIFI_RUNTIME_WAIT_TIMEOUT_MS = 10000;
static const uint32_t NTP_RUNTIME_WAIT_TIMEOUT_MS = 10000;
static const uint32_t DEFAULT_MANUAL_OVERRIDE_SECONDS = 5 * 60;
static const uint32_t FALLBACK_SLEEP_WAIT_SECONDS = 60;
static const char *SLEEP_TAG = "app_sleep";

static void app_runtime_update_ui_network_status(app_bootstrap_context_t *context);

static void app_runtime_set_mode(app_bootstrap_context_t *context, app_runtime_mode_t runtime_mode)
{
    if (context == NULL)
    {
        return;
    }

    context->runtime_mode = runtime_mode;
    ui_boot_model_set_runtime_mode(&context->ui_boot_model, runtime_mode);
    if (runtime_mode != APP_RUNTIME_MODE_MANUAL_ACTIVE_OVERRIDE)
    {
        context->manual_override_until_monotonic = 0;
        context->manual_override_until_epoch = 0;
    }

    if (runtime_mode == APP_RUNTIME_MODE_SCHEDULED_SLEEP)
    {
        (void)wifi_platform_stop();
        app_runtime_update_ui_network_status(context);
    }
}

static uint32_t app_runtime_manual_override_seconds(const app_bootstrap_context_t *context)
{
    uint32_t minutes;

    if (context == NULL)
    {
        return DEFAULT_MANUAL_OVERRIDE_SECONDS;
    }

    minutes = context->config.device.sleep_schedule.manual_wake_minutes;
    if (minutes == 0)
    {
        return 0;
    }

    return minutes * 60U;
}

static bool app_epoch_to_rtc_time(int64_t epoch_seconds, rtc_time_t *rtc_time)
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

static bool app_rtc_time_to_epoch(const rtc_time_t *rtc_time, int64_t *epoch_seconds)
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

static bool app_rtc_add_seconds(const rtc_time_t *base_time, uint32_t delta_seconds, rtc_time_t *result)
{
    int64_t epoch_seconds;

    if (result == NULL || !app_rtc_time_to_epoch(base_time, &epoch_seconds))
    {
        return false;
    }

    return app_epoch_to_rtc_time(epoch_seconds + (int64_t)delta_seconds, result);
}

static void app_runtime_log_sleep_transition(const app_bootstrap_context_t *context,
                                             const char *event_text,
                                             uint32_t seconds_until_wake)
{
    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();

    if (context == NULL)
    {
        return;
    }

    ESP_LOGI(SLEEP_TAG,
             "%s mode=%d rtc=%04u-%02u-%02u %02u:%02u:%02u wake_in=%us wake_cause=%d wifi=%s",
             event_text,
             (int)context->runtime_mode,
             context->rtc_time.year,
             context->rtc_time.month,
             context->rtc_time.day,
             context->rtc_time.hour,
             context->rtc_time.minute,
             context->rtc_time.second,
             (unsigned)seconds_until_wake,
             (int)wakeup_cause,
             wifi_platform_status_text());
}

static bool app_runtime_prepare_deep_sleep(const app_bootstrap_context_t *context,
                                           uint32_t *seconds_until_wake)
{
    const board_config_t *board = board_get_config();
    uint32_t transition_seconds;
    bool next_is_sleep = false;

    if (context == NULL || seconds_until_wake == NULL || board == NULL || !context->rtc_time.valid)
    {
        return false;
    }

    if (!app_sleep_schedule_seconds_until_transition(&context->config,
                                                     &context->rtc_time,
                                                     &transition_seconds,
                                                     &next_is_sleep))
    {
        return false;
    }

    if (next_is_sleep)
    {
        return false;
    }

    ESP_ERROR_CHECK(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL));
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup((uint64_t)transition_seconds * 1000000ULL));
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup_io(1ULL << board->buttons.user_gpio,
                                                    board->buttons.active_low ? ESP_EXT1_WAKEUP_ANY_LOW
                                                                              : ESP_EXT1_WAKEUP_ANY_HIGH));

    *seconds_until_wake = transition_seconds;
    return true;
}

static void app_runtime_enter_scheduled_deep_sleep(app_bootstrap_context_t *context)
{
    uint32_t seconds_until_wake = 0;

    if (context == NULL)
    {
        return;
    }

    if (!app_runtime_prepare_deep_sleep(context, &seconds_until_wake))
    {
        ESP_LOGW(SLEEP_TAG, "deep sleep preparation failed; staying in scheduled sleep loop");
        return;
    }

    app_runtime_log_sleep_transition(context, "entering deep sleep", seconds_until_wake);
    esp_deep_sleep_start();
}

static bool app_runtime_refresh_rtc(app_bootstrap_context_t *context)
{
    if (context == NULL)
    {
        return false;
    }

    if (!sensor_service_read_rtc(&context->rtc_time))
    {
        return false;
    }

    ui_boot_model_set_rtc(&context->ui_boot_model, &context->rtc_time);
    return true;
}

static void app_runtime_update_ui_network_status(app_bootstrap_context_t *context)
{
    int8_t rssi_dbm = 0;
    bool has_rssi;

    if (context == NULL)
    {
        return;
    }

    has_rssi = wifi_platform_get_rssi_dbm(&rssi_dbm);
    ui_boot_model_set_wifi_status(&context->ui_boot_model, wifi_platform_is_ready(), has_rssi, rssi_dbm);
}

static void app_runtime_update_ui_provider_timing(app_bootstrap_context_t *context,
                                                  bool provider_request_ok)
{
    rtc_time_t next_attempt_time;
    uint32_t next_interval_seconds;

    if (context == NULL || !context->rtc_time.valid)
    {
        return;
    }

    if (provider_request_ok)
    {
        ui_boot_model_set_provider_last_sync_time(&context->ui_boot_model, &context->rtc_time);
    }

    next_interval_seconds = provider_poll_state_next_interval_seconds(&context->provider_poll_state);
    if (next_interval_seconds > 0 && app_rtc_add_seconds(&context->rtc_time, next_interval_seconds, &next_attempt_time))
    {
        ui_boot_model_set_provider_next_attempt_time(&context->ui_boot_model, &next_attempt_time);
    }
}

static void app_runtime_set_provider_network_placeholder(const provider_interface_t *active_provider,
                                                         const provider_config_t *active_config,
                                                         provider_snapshot_t *snapshot)
{
    if (active_provider == NULL || active_config == NULL || snapshot == NULL)
    {
        return;
    }

    provider_snapshot_init(snapshot);
    snapshot->provider_type = active_provider->provider_type;
    snapshot->sync_state = PROVIDER_SYNC_STATE_NETWORK_ERROR;
    snapshot->stale = true;
    snapshot->capabilities.supports_multiple_windows = true;
    snapshot->capabilities.supports_manual_refresh = true;
    snapshot->capabilities.supports_region_label = true;
    snprintf(snapshot->provider_id, sizeof(snapshot->provider_id), "%s", active_config->id);
    snprintf(snapshot->display_name, sizeof(snapshot->display_name), "%s", active_provider->display_name);
    snprintf(snapshot->region, sizeof(snapshot->region), "%s", active_config->region);
    snapshot->metric_count = 1;
    snprintf(snapshot->metrics[0].label, sizeof(snapshot->metrics[0].label), "transport");
    snprintf(snapshot->metrics[0].value_text,
             sizeof(snapshot->metrics[0].value_text),
             "%s",
             wifi_platform_status_text());
    snapshot->metrics[0].priority = 1;
}

static uint32_t app_runtime_compute_wait_seconds(const app_bootstrap_context_t *context, int64_t now_monotonic_seconds)
{
    uint32_t wait_seconds;
    uint32_t transition_seconds;

    if (context == NULL)
    {
        return 1;
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_SCHEDULED_SLEEP)
    {
        if (context->rtc_time.valid &&
            app_sleep_schedule_seconds_until_transition(&context->config,
                                                        &context->rtc_time,
                                                        &transition_seconds,
                                                        NULL))
        {
            return transition_seconds > 0 ? transition_seconds : 1;
        }

        return FALLBACK_SLEEP_WAIT_SECONDS;
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_WAKING)
    {
        return 1;
    }

    wait_seconds = app_scheduler_next_wake_delay_seconds(&context->scheduler_state,
                                                         now_monotonic_seconds,
                                                         &context->provider_poll_state,
                                                         time_service_get_state());

    if (context->rtc_time.valid &&
        app_sleep_schedule_seconds_until_transition(&context->config,
                                                    &context->rtc_time,
                                                    &transition_seconds,
                                                    NULL) &&
        transition_seconds < wait_seconds)
    {
        wait_seconds = transition_seconds;
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_MANUAL_ACTIVE_OVERRIDE)
    {
        if (context->manual_override_until_monotonic == 0)
        {
            return wait_seconds > 0 ? wait_seconds : 1;
        }

        if (context->manual_override_until_monotonic <= now_monotonic_seconds)
        {
            return 1;
        }

        transition_seconds = (uint32_t)(context->manual_override_until_monotonic - now_monotonic_seconds);
        if (transition_seconds < wait_seconds)
        {
            wait_seconds = transition_seconds;
        }
    }

    return wait_seconds > 0 ? wait_seconds : 1;
}

static void app_runtime_wait_until_next_due(app_bootstrap_context_t *context)
{
    TickType_t wait_ticks;
    uint32_t wait_seconds;

    if (context == NULL)
    {
        return;
    }

    wait_seconds = app_runtime_compute_wait_seconds(context, esp_timer_get_time() / 1000000);
    wait_ticks = pdMS_TO_TICKS(wait_seconds * 1000U);
    if (wait_ticks == 0)
    {
        wait_ticks = 1;
    }

    (void)ulTaskNotifyTake(pdTRUE, wait_ticks);
}

static void app_runtime_apply_schedule_state(app_bootstrap_context_t *context,
                                             int64_t now_monotonic_seconds,
                                             bool key_pressed,
                                             bool *force_refresh,
                                             bool *ui_needs_update)
{
    bool active_by_schedule = true;
    bool have_schedule_state;

    if (context == NULL || force_refresh == NULL || ui_needs_update == NULL)
    {
        return;
    }

    have_schedule_state = app_sleep_schedule_is_active_now(&context->config,
                                                           &context->rtc_time,
                                                           &active_by_schedule);

    if (context->runtime_mode == APP_RUNTIME_MODE_MANUAL_ACTIVE_OVERRIDE)
    {
        if (have_schedule_state && active_by_schedule)
        {
            app_runtime_set_mode(context, APP_RUNTIME_MODE_SCHEDULED_ACTIVE);
            *ui_needs_update = true;
        }
        else if (context->manual_override_until_epoch > 0)
        {
            int64_t now_wall_epoch;

            if (app_rtc_time_to_epoch(&context->rtc_time, &now_wall_epoch) &&
                now_wall_epoch >= context->manual_override_until_epoch)
            {
                app_runtime_set_mode(context,
                                     (have_schedule_state && !active_by_schedule)
                                         ? APP_RUNTIME_MODE_SCHEDULED_SLEEP
                                         : APP_RUNTIME_MODE_SCHEDULED_ACTIVE);
                *ui_needs_update = true;
            }
        }
        else if (context->manual_override_until_monotonic > 0 &&
                 now_monotonic_seconds >= context->manual_override_until_monotonic)
        {
            app_runtime_set_mode(context,
                                 (have_schedule_state && !active_by_schedule)
                                     ? APP_RUNTIME_MODE_SCHEDULED_SLEEP
                                     : APP_RUNTIME_MODE_SCHEDULED_ACTIVE);
            *ui_needs_update = true;
        }
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_SCHEDULED_SLEEP)
    {
        if (have_schedule_state && active_by_schedule)
        {
            app_runtime_set_mode(context, APP_RUNTIME_MODE_SCHEDULED_ACTIVE);
            *force_refresh = true;
            *ui_needs_update = true;
        }
    }
    else if (context->runtime_mode == APP_RUNTIME_MODE_SCHEDULED_ACTIVE)
    {
        if (have_schedule_state && !active_by_schedule)
        {
            app_runtime_set_mode(context, APP_RUNTIME_MODE_SCHEDULED_SLEEP);
            *ui_needs_update = true;
        }
    }

    if (!key_pressed)
    {
        return;
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_SCHEDULED_SLEEP)
    {
        app_runtime_set_mode(context, APP_RUNTIME_MODE_WAKING);
        if (app_runtime_manual_override_seconds(context) > 0)
        {
            uint32_t override_seconds = app_runtime_manual_override_seconds(context);
            int64_t now_wall_epoch;

            context->manual_override_until_monotonic = now_monotonic_seconds + override_seconds;
            if (app_rtc_time_to_epoch(&context->rtc_time, &now_wall_epoch))
            {
                context->manual_override_until_epoch = now_wall_epoch + (int64_t)override_seconds;
            }
        }
        *force_refresh = true;
        *ui_needs_update = true;
        return;
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_MANUAL_ACTIVE_OVERRIDE)
    {
        if (app_runtime_manual_override_seconds(context) > 0)
        {
            uint32_t override_seconds = app_runtime_manual_override_seconds(context);
            int64_t now_wall_epoch;

            context->manual_override_until_monotonic = now_monotonic_seconds + override_seconds;
            if (app_rtc_time_to_epoch(&context->rtc_time, &now_wall_epoch))
            {
                context->manual_override_until_epoch = now_wall_epoch + (int64_t)override_seconds;
            }
        }
    }

    *force_refresh = true;
}

static void app_runtime_apply_post_work_schedule(app_bootstrap_context_t *context, bool *ui_needs_update)
{
    bool active_by_schedule = true;

    if (context == NULL || ui_needs_update == NULL || !context->rtc_time.valid)
    {
        return;
    }

    if (!app_sleep_schedule_is_active_now(&context->config, &context->rtc_time, &active_by_schedule))
    {
        return;
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_SCHEDULED_ACTIVE && !active_by_schedule)
    {
        app_runtime_set_mode(context, APP_RUNTIME_MODE_SCHEDULED_SLEEP);
        *ui_needs_update = true;
        return;
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_MANUAL_ACTIVE_OVERRIDE && active_by_schedule)
    {
        app_runtime_set_mode(context, APP_RUNTIME_MODE_SCHEDULED_ACTIVE);
        *ui_needs_update = true;
    }
}

static void app_runtime_run_active_cycle(app_bootstrap_context_t *context,
                                         int64_t now_monotonic_seconds,
                                         bool force_refresh,
                                         bool *ui_needs_update)
{
    app_scheduler_due_t due;
    bool network_work_due;
    bool network_ready = false;
    bool provider_request_attempted = false;
    bool provider_request_ok = false;

    if (context == NULL || ui_needs_update == NULL)
    {
        return;
    }

    app_scheduler_compute_due(&context->scheduler_state,
                              now_monotonic_seconds,
                              &context->provider_poll_state,
                              time_service_get_state(),
                              &due);

    if (force_refresh)
    {
        due.clock_due = true;
        due.environment_due = true;
        due.power_due = true;
        due.provider_due = provider_registry_has_active();
        provider_poll_state_note_manual_refresh(&context->provider_poll_state);
    }

    network_work_due = (due.provider_due && provider_registry_has_active()) || due.ntp_due;
    if (network_work_due)
    {
        network_ready = wifi_platform_ensure_ready(&context->config, WIFI_RUNTIME_WAIT_TIMEOUT_MS);
        app_runtime_update_ui_network_status(context);
        *ui_needs_update = true;
    }

    if (due.clock_due)
    {
        if (app_runtime_refresh_rtc(context))
        {
            app_scheduler_note_clock_rendered(&context->scheduler_state, now_monotonic_seconds);
            *ui_needs_update = true;
        }
    }

    if (due.environment_due)
    {
        if (sensor_service_read_environment(&context->environment))
        {
            ui_boot_model_set_environment(&context->ui_boot_model, &context->environment);
            app_scheduler_note_environment_polled(&context->scheduler_state, now_monotonic_seconds);
            *ui_needs_update = true;
        }
    }

    if (due.power_due)
    {
        if (power_service_get_status(&context->power_status))
        {
            ui_boot_model_set_power(&context->ui_boot_model, &context->power_status);
            *ui_needs_update = true;
        }

        app_scheduler_note_power_polled(&context->scheduler_state, now_monotonic_seconds);
    }

    if (due.provider_due && provider_registry_has_active())
    {
        bool changed = false;
        const provider_interface_t *active_provider = provider_registry_get_active();
        const provider_config_t *active_provider_config = provider_registry_get_active_config();

        provider_request_attempted = true;

        if (!network_ready)
        {
            app_runtime_set_provider_network_placeholder(active_provider,
                                                         active_provider_config,
                                                         &context->boot_snapshot);
            ui_boot_model_set_provider_snapshot(&context->ui_boot_model, &context->boot_snapshot);
            *ui_needs_update = true;
            provider_poll_state_note_failure(&context->provider_poll_state);
            provider_request_ok = false;
            ESP_LOGW(TAG, "provider refresh skipped: %s", wifi_platform_status_text());
        }
        else if (provider_registry_fetch_active_snapshot(&context->boot_snapshot))
        {
            ui_boot_model_set_provider_snapshot(&context->ui_boot_model, &context->boot_snapshot);
            *ui_needs_update = true;

            if (context->boot_snapshot.sync_state == PROVIDER_SYNC_STATE_OK)
            {
                provider_request_ok = true;
                (void)provider_poll_state_note_success(&context->provider_poll_state,
                                                       &context->boot_snapshot,
                                                       false,
                                                       &changed);
                ESP_LOGI(TAG,
                         "provider refresh sync=%d changed=%s next=%us",
                         (int)context->boot_snapshot.sync_state,
                         changed ? "yes" : "no",
                         provider_poll_state_next_interval_seconds(&context->provider_poll_state));
            }
            else
            {
                provider_request_ok = false;
                provider_poll_state_note_failure(&context->provider_poll_state);
                ESP_LOGW(TAG,
                         "provider refresh sync=%d next=%us",
                         (int)context->boot_snapshot.sync_state,
                         provider_poll_state_next_interval_seconds(&context->provider_poll_state));
            }
        }
        else
        {
            provider_request_ok = false;
            provider_poll_state_note_failure(&context->provider_poll_state);
        }

        app_scheduler_note_provider_polled(&context->scheduler_state, now_monotonic_seconds);
    }

    if (provider_request_attempted && app_runtime_refresh_rtc(context))
    {
        app_runtime_update_ui_provider_timing(context, provider_request_ok);
        *ui_needs_update = true;
    }

    if (due.weather_due)
    {
        app_scheduler_note_weather_polled(&context->scheduler_state, now_monotonic_seconds);
        ESP_LOGI(TAG, "weather refresh due");
    }

    if (due.ntp_due)
    {
        int64_t synced_epoch;

        ESP_LOGI(TAG, "ntp sync due");
        if (network_ready && time_service_sync_ntp(NTP_RUNTIME_WAIT_TIMEOUT_MS, now_monotonic_seconds, &synced_epoch) &&
            app_epoch_to_rtc_time(synced_epoch, &context->rtc_time))
        {
            (void)sensor_service_write_rtc(&context->rtc_time);
            ui_boot_model_set_rtc(&context->ui_boot_model, &context->rtc_time);
            app_scheduler_note_clock_rendered(&context->scheduler_state, now_monotonic_seconds);
            *ui_needs_update = true;
        }
        app_scheduler_note_ntp_attempted(&context->scheduler_state, now_monotonic_seconds);
    }

    if (network_work_due && context->config.wifi.enabled)
    {
        (void)wifi_platform_stop();
        app_runtime_update_ui_network_status(context);
        *ui_needs_update = true;
    }
}

static void app_runtime_task(void *arg)
{
    app_bootstrap_context_t *context = (app_bootstrap_context_t *)arg;

    app_key_input_register_task(xTaskGetCurrentTaskHandle());

    while (true)
    {
        app_runtime_step(context, esp_timer_get_time() / 1000000);
        app_runtime_wait_until_next_due(context);
    }
}

void app_runtime_step(app_bootstrap_context_t *context, int64_t now_epoch_seconds)
{
    bool ui_needs_update = false;
    bool force_refresh = false;
    bool key_pressed;

    if (context == NULL)
    {
        return;
    }

    (void)app_runtime_refresh_rtc(context);

    key_pressed = app_key_input_consume_press();
    app_runtime_apply_schedule_state(context,
                                     now_epoch_seconds,
                                     key_pressed,
                                     &force_refresh,
                                     &ui_needs_update);

    if (context->runtime_mode == APP_RUNTIME_MODE_WAKING)
    {
        if (ui_needs_update)
        {
            ui_app_update(&context->ui_boot_model);
            ui_needs_update = false;
        }

        app_runtime_set_mode(context, APP_RUNTIME_MODE_MANUAL_ACTIVE_OVERRIDE);
        force_refresh = true;

        if (app_runtime_manual_override_seconds(context) > 0 &&
            context->manual_override_until_monotonic == 0)
        {
            uint32_t override_seconds = app_runtime_manual_override_seconds(context);
            int64_t now_wall_epoch;

            context->manual_override_until_monotonic = now_epoch_seconds + override_seconds;
            if (app_rtc_time_to_epoch(&context->rtc_time, &now_wall_epoch))
            {
                context->manual_override_until_epoch = now_wall_epoch + (int64_t)override_seconds;
            }
        }
    }

    if (context->runtime_mode == APP_RUNTIME_MODE_SCHEDULED_SLEEP)
    {
        if (ui_needs_update)
        {
            ui_app_update(&context->ui_boot_model);
        }
        app_runtime_enter_scheduled_deep_sleep(context);
        return;
    }

    app_runtime_run_active_cycle(context,
                                 now_epoch_seconds,
                                 force_refresh,
                                 &ui_needs_update);

    (void)app_runtime_refresh_rtc(context);
    app_runtime_apply_post_work_schedule(context, &ui_needs_update);

    if (ui_needs_update)
    {
        ui_app_update(&context->ui_boot_model);
    }
}

bool app_runtime_start(app_bootstrap_context_t *context)
{
    BaseType_t result;

    if (context == NULL)
    {
        return false;
    }

    result = xTaskCreate(app_runtime_task, "app_runtime", APP_RUNTIME_TASK_STACK_SIZE, context, 5, NULL);
    return result == pdPASS;
}