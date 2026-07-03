#include "refresh_policy.h"

enum
{
    REFRESH_INTERVAL_HIGH_SECONDS = 5 * 60,
    REFRESH_INTERVAL_NORMAL_SECONDS = 15 * 60,
    REFRESH_INTERVAL_LOW_SECONDS = 30 * 60,
    REFRESH_RETRY_FIRST_SECONDS = 10 * 60,
    REFRESH_RETRY_REPEAT_SECONDS = 30 * 60,
    REFRESH_RETRY_MAX_SECONDS = 60 * 60,
};

void provider_refresh_policy_init(provider_refresh_policy_t *policy)
{
    if (policy == NULL)
    {
        return;
    }

    policy->band = PROVIDER_REFRESH_BAND_NORMAL;
    policy->changed_streak = 0;
    policy->unchanged_streak = 0;
    policy->failure_streak = 0;
    policy->last_snapshot_fingerprint = 0;
}

void provider_refresh_policy_note_success(provider_refresh_policy_t *policy, bool changed, bool reset_boundary_soon)
{
    if (policy == NULL)
    {
        return;
    }

    policy->failure_streak = 0;

    if (changed)
    {
        if (policy->changed_streak < UINT8_MAX)
        {
            ++policy->changed_streak;
        }
        policy->unchanged_streak = 0;
    }
    else
    {
        if (policy->unchanged_streak < UINT8_MAX)
        {
            ++policy->unchanged_streak;
        }
        policy->changed_streak = 0;
    }

    if (reset_boundary_soon || policy->changed_streak >= 2)
    {
        policy->band = PROVIDER_REFRESH_BAND_HIGH;
        return;
    }

    if (policy->unchanged_streak >= 2)
    {
        policy->band = PROVIDER_REFRESH_BAND_LOW;
        return;
    }

    policy->band = PROVIDER_REFRESH_BAND_NORMAL;
}

void provider_refresh_policy_note_failure(provider_refresh_policy_t *policy)
{
    if (policy == NULL)
    {
        return;
    }

    policy->changed_streak = 0;
    policy->unchanged_streak = 0;
    if (policy->failure_streak < UINT8_MAX)
    {
        ++policy->failure_streak;
    }
}

void provider_refresh_policy_force_high(provider_refresh_policy_t *policy)
{
    if (policy == NULL)
    {
        return;
    }

    policy->band = PROVIDER_REFRESH_BAND_HIGH;
    policy->changed_streak = 0;
    policy->unchanged_streak = 0;
    policy->failure_streak = 0;
}

uint32_t provider_refresh_policy_get_interval_seconds(const provider_refresh_policy_t *policy)
{
    if (policy == NULL)
    {
        return REFRESH_INTERVAL_NORMAL_SECONDS;
    }

    if (policy->failure_streak > 0)
    {
        if (policy->failure_streak == 1)
        {
            return REFRESH_RETRY_FIRST_SECONDS;
        }
        if (policy->failure_streak == 2)
        {
            return REFRESH_RETRY_REPEAT_SECONDS;
        }
        return REFRESH_RETRY_MAX_SECONDS;
    }

    switch (policy->band)
    {
    case PROVIDER_REFRESH_BAND_HIGH:
        return REFRESH_INTERVAL_HIGH_SECONDS;
    case PROVIDER_REFRESH_BAND_LOW:
        return REFRESH_INTERVAL_LOW_SECONDS;
    case PROVIDER_REFRESH_BAND_NORMAL:
    default:
        return REFRESH_INTERVAL_NORMAL_SECONDS;
    }
}