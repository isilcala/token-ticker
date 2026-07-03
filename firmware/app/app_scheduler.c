#include "app_scheduler.h"

#include <limits.h>
#include <string.h>

enum
{
    ENVIRONMENT_REFRESH_INTERVAL_SECONDS = 15 * 60,
    POWER_REFRESH_INTERVAL_SECONDS = 30 * 60,
    WEATHER_REFRESH_INTERVAL_SECONDS = 60 * 60,
    NTP_RETRY_INTERVAL_SECONDS = 30 * 60,
};

void app_scheduler_init(app_scheduler_state_t *state, bool weather_enabled)
{
    if (state == NULL)
    {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->weather_enabled = weather_enabled;
    state->last_clock_minute = -1;
}

void app_scheduler_compute_due(const app_scheduler_state_t *state,
                               int64_t now_epoch_seconds,
                               const provider_poll_state_t *provider_poll_state,
                               const time_service_state_t *time_state,
                               app_scheduler_due_t *due)
{
    int64_t current_minute;
    uint32_t provider_interval_seconds;

    if (state == NULL || due == NULL)
    {
        return;
    }

    memset(due, 0, sizeof(*due));

    if (now_epoch_seconds < 0)
    {
        return;
    }

    current_minute = now_epoch_seconds / 60;
    due->clock_due = current_minute != state->last_clock_minute;

    provider_interval_seconds = provider_poll_state_next_interval_seconds(provider_poll_state);
    due->provider_due = (state->last_provider_poll_epoch == 0) || (provider_interval_seconds > 0 && (now_epoch_seconds - state->last_provider_poll_epoch) >= (int64_t)provider_interval_seconds);

    due->environment_due = (state->last_environment_poll_epoch == 0) || ((now_epoch_seconds - state->last_environment_poll_epoch) >= ENVIRONMENT_REFRESH_INTERVAL_SECONDS);

    due->power_due = (state->last_power_poll_epoch == 0) || ((now_epoch_seconds - state->last_power_poll_epoch) >= POWER_REFRESH_INTERVAL_SECONDS);

    due->weather_due = state->weather_enabled && ((state->last_weather_poll_epoch == 0) || ((now_epoch_seconds - state->last_weather_poll_epoch) >= WEATHER_REFRESH_INTERVAL_SECONDS));

    due->ntp_due = time_service_should_sync_ntp(now_epoch_seconds) && ((state->last_ntp_attempt_epoch == 0) || ((now_epoch_seconds - state->last_ntp_attempt_epoch) >= NTP_RETRY_INTERVAL_SECONDS));
}

uint32_t app_scheduler_next_wake_delay_seconds(const app_scheduler_state_t *state,
                                               int64_t now_epoch_seconds,
                                               const provider_poll_state_t *provider_poll_state,
                                               const time_service_state_t *time_state)
{
    app_scheduler_due_t due;
    int64_t next_due_epoch = INT64_MAX;
    uint32_t provider_interval_seconds;
    int64_t ntp_interval_seconds;
    int64_t ntp_sync_epoch;
    int64_t ntp_retry_epoch;

    if (state == NULL || now_epoch_seconds < 0)
    {
        return 1;
    }

    app_scheduler_compute_due(state,
                              now_epoch_seconds,
                              provider_poll_state,
                              time_state,
                              &due);

    if (due.clock_due || due.provider_due || due.environment_due || due.power_due || due.weather_due || due.ntp_due)
    {
        return 1;
    }

    if (state->last_clock_minute >= 0)
    {
        next_due_epoch = ((state->last_clock_minute + 1) * 60);
    }

    provider_interval_seconds = provider_poll_state_next_interval_seconds(provider_poll_state);
    if (provider_interval_seconds > 0 && state->last_provider_poll_epoch > 0)
    {
        const int64_t provider_due_epoch = state->last_provider_poll_epoch + (int64_t)provider_interval_seconds;
        if (provider_due_epoch < next_due_epoch)
        {
            next_due_epoch = provider_due_epoch;
        }
    }

    if (state->last_environment_poll_epoch > 0)
    {
        const int64_t environment_due_epoch = state->last_environment_poll_epoch + ENVIRONMENT_REFRESH_INTERVAL_SECONDS;
        if (environment_due_epoch < next_due_epoch)
        {
            next_due_epoch = environment_due_epoch;
        }
    }

    if (state->last_power_poll_epoch > 0)
    {
        const int64_t power_due_epoch = state->last_power_poll_epoch + POWER_REFRESH_INTERVAL_SECONDS;
        if (power_due_epoch < next_due_epoch)
        {
            next_due_epoch = power_due_epoch;
        }
    }

    if (state->weather_enabled && state->last_weather_poll_epoch > 0)
    {
        const int64_t weather_due_epoch = state->last_weather_poll_epoch + WEATHER_REFRESH_INTERVAL_SECONDS;
        if (weather_due_epoch < next_due_epoch)
        {
            next_due_epoch = weather_due_epoch;
        }
    }

    if (time_state != NULL && time_state->ntp_enabled)
    {
        ntp_interval_seconds = (int64_t)time_state->ntp_sync_hours * 3600;
        ntp_sync_epoch = (time_state->last_ntp_sync_monotonic_epoch <= 0)
                             ? now_epoch_seconds
                             : time_state->last_ntp_sync_monotonic_epoch + ntp_interval_seconds;
        ntp_retry_epoch = (state->last_ntp_attempt_epoch <= 0)
                              ? now_epoch_seconds
                              : state->last_ntp_attempt_epoch + NTP_RETRY_INTERVAL_SECONDS;

        if (ntp_sync_epoch < ntp_retry_epoch)
        {
            ntp_sync_epoch = ntp_retry_epoch;
        }

        if (ntp_sync_epoch < next_due_epoch)
        {
            next_due_epoch = ntp_sync_epoch;
        }
    }

    if (next_due_epoch == INT64_MAX || next_due_epoch <= now_epoch_seconds)
    {
        return 1;
    }

    return (uint32_t)(next_due_epoch - now_epoch_seconds);
}

void app_scheduler_note_clock_rendered(app_scheduler_state_t *state, int64_t now_epoch_seconds)
{
    if (state == NULL || now_epoch_seconds < 0)
    {
        return;
    }

    state->last_clock_minute = now_epoch_seconds / 60;
}

void app_scheduler_note_provider_polled(app_scheduler_state_t *state, int64_t now_epoch_seconds)
{
    if (state == NULL || now_epoch_seconds < 0)
    {
        return;
    }

    state->last_provider_poll_epoch = now_epoch_seconds;
}

void app_scheduler_note_environment_polled(app_scheduler_state_t *state, int64_t now_epoch_seconds)
{
    if (state == NULL || now_epoch_seconds < 0)
    {
        return;
    }

    state->last_environment_poll_epoch = now_epoch_seconds;
}

void app_scheduler_note_power_polled(app_scheduler_state_t *state, int64_t now_epoch_seconds)
{
    if (state == NULL || now_epoch_seconds < 0)
    {
        return;
    }

    state->last_power_poll_epoch = now_epoch_seconds;
}

void app_scheduler_note_weather_polled(app_scheduler_state_t *state, int64_t now_epoch_seconds)
{
    if (state == NULL || now_epoch_seconds < 0)
    {
        return;
    }

    state->last_weather_poll_epoch = now_epoch_seconds;
}

void app_scheduler_note_ntp_attempted(app_scheduler_state_t *state, int64_t now_epoch_seconds)
{
    if (state == NULL || now_epoch_seconds < 0)
    {
        return;
    }

    state->last_ntp_attempt_epoch = now_epoch_seconds;
}