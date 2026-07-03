#include "provider_poll_state.h"

#include <string.h>

#include "snapshot_fingerprint.h"

void provider_poll_state_init(provider_poll_state_t *state)
{
    if (state == NULL)
    {
        return;
    }

    memset(state, 0, sizeof(*state));
    provider_refresh_policy_init(&state->refresh_policy);
}

void provider_poll_state_note_manual_refresh(provider_poll_state_t *state)
{
    if (state == NULL)
    {
        return;
    }

    provider_refresh_policy_force_high(&state->refresh_policy);
}

bool provider_poll_state_note_success(provider_poll_state_t *state,
                                      const provider_snapshot_t *snapshot,
                                      bool reset_boundary_soon,
                                      bool *changed)
{
    uint32_t fingerprint;
    bool did_change;

    if (state == NULL || snapshot == NULL)
    {
        return false;
    }

    fingerprint = provider_snapshot_fingerprint(snapshot);
    did_change = !state->has_last_snapshot || fingerprint != state->last_fingerprint;

    provider_refresh_policy_note_success(&state->refresh_policy, did_change, reset_boundary_soon);

    state->last_snapshot = *snapshot;
    state->last_fingerprint = fingerprint;
    state->has_last_snapshot = true;

    if (changed != NULL)
    {
        *changed = did_change;
    }

    return true;
}

void provider_poll_state_note_failure(provider_poll_state_t *state)
{
    if (state == NULL)
    {
        return;
    }

    provider_refresh_policy_note_failure(&state->refresh_policy);
}

uint32_t provider_poll_state_next_interval_seconds(const provider_poll_state_t *state)
{
    if (state == NULL)
    {
        return 0;
    }

    return provider_refresh_policy_get_interval_seconds(&state->refresh_policy);
}