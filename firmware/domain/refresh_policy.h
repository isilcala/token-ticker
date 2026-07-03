#ifndef TOKEN_TICKER_REFRESH_POLICY_H
#define TOKEN_TICKER_REFRESH_POLICY_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    PROVIDER_REFRESH_BAND_HIGH = 0,
    PROVIDER_REFRESH_BAND_NORMAL,
    PROVIDER_REFRESH_BAND_LOW,
} provider_refresh_band_t;

typedef struct
{
    provider_refresh_band_t band;
    uint8_t changed_streak;
    uint8_t unchanged_streak;
    uint8_t failure_streak;
    uint32_t last_snapshot_fingerprint;
} provider_refresh_policy_t;

void provider_refresh_policy_init(provider_refresh_policy_t *policy);
void provider_refresh_policy_note_success(provider_refresh_policy_t *policy, bool changed, bool reset_boundary_soon);
void provider_refresh_policy_note_failure(provider_refresh_policy_t *policy);
void provider_refresh_policy_force_high(provider_refresh_policy_t *policy);
uint32_t provider_refresh_policy_get_interval_seconds(const provider_refresh_policy_t *policy);

#endif