#ifndef TOKEN_TICKER_PROVIDER_POLL_STATE_H
#define TOKEN_TICKER_PROVIDER_POLL_STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "provider_snapshot.h"
#include "refresh_policy.h"

typedef struct
{
    bool has_last_snapshot;
    uint32_t last_fingerprint;
    provider_snapshot_t last_snapshot;
    provider_refresh_policy_t refresh_policy;
} provider_poll_state_t;

void provider_poll_state_init(provider_poll_state_t *state);
void provider_poll_state_note_manual_refresh(provider_poll_state_t *state);
bool provider_poll_state_note_success(provider_poll_state_t *state,
                                      const provider_snapshot_t *snapshot,
                                      bool reset_boundary_soon,
                                      bool *changed);
void provider_poll_state_note_failure(provider_poll_state_t *state);
uint32_t provider_poll_state_next_interval_seconds(const provider_poll_state_t *state);

#endif