#include "snapshot_fingerprint.h"

static uint32_t fnv1a_u8(uint32_t hash, uint8_t value)
{
    return (hash ^ value) * 16777619u;
}

static uint32_t fnv1a_i32(uint32_t hash, int32_t value)
{
    uint32_t raw = (uint32_t)value;
    hash = fnv1a_u8(hash, (uint8_t)(raw & 0xFFu));
    hash = fnv1a_u8(hash, (uint8_t)((raw >> 8) & 0xFFu));
    hash = fnv1a_u8(hash, (uint8_t)((raw >> 16) & 0xFFu));
    hash = fnv1a_u8(hash, (uint8_t)((raw >> 24) & 0xFFu));
    return hash;
}

static uint32_t fnv1a_i64(uint32_t hash, int64_t value)
{
    uint64_t raw = (uint64_t)value;
    uint8_t shift;

    for (shift = 0; shift < 8; ++shift)
    {
        hash = fnv1a_u8(hash, (uint8_t)((raw >> (shift * 8)) & 0xFFu));
    }

    return hash;
}

static uint32_t fnv1a_str(uint32_t hash, const char *value)
{
    const unsigned char *cursor = (const unsigned char *)value;

    if (value == NULL)
    {
        return fnv1a_u8(hash, 0);
    }

    while (*cursor != '\0')
    {
        hash = fnv1a_u8(hash, *cursor);
        ++cursor;
    }

    return fnv1a_u8(hash, 0);
}

uint32_t provider_snapshot_fingerprint(const provider_snapshot_t *snapshot)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (snapshot == NULL)
    {
        return 0;
    }

    hash = fnv1a_str(hash, snapshot->provider_id);
    hash = fnv1a_str(hash, snapshot->display_name);
    hash = fnv1a_str(hash, snapshot->region);
    hash = fnv1a_i32(hash, (int32_t)snapshot->provider_type);
    hash = fnv1a_i32(hash, (int32_t)snapshot->sync_state);
    hash = fnv1a_u8(hash, snapshot->stale ? 1u : 0u);
    hash = fnv1a_i64(hash, snapshot->last_success_epoch);
    hash = fnv1a_u8(hash, snapshot->primary_window_index);
    hash = fnv1a_i32(hash, (int32_t)snapshot->window_count);

    for (index = 0; index < snapshot->window_count && index < PROVIDER_SNAPSHOT_MAX_WINDOWS; ++index)
    {
        const quota_window_t *window = &snapshot->windows[index];
        hash = fnv1a_str(hash, window->id);
        hash = fnv1a_str(hash, window->label);
        hash = fnv1a_i32(hash, window->used);
        hash = fnv1a_i32(hash, window->limit);
        hash = fnv1a_i32(hash, window->remaining);
        hash = fnv1a_i32(hash, window->remaining_percent_x100);
        hash = fnv1a_i64(hash, window->start_epoch);
        hash = fnv1a_i64(hash, window->reset_epoch);
        hash = fnv1a_i32(hash, (int32_t)window->status);
    }

    hash = fnv1a_i32(hash, (int32_t)snapshot->metric_count);
    for (index = 0; index < snapshot->metric_count && index < PROVIDER_SNAPSHOT_MAX_METRICS; ++index)
    {
        const detail_metric_t *metric = &snapshot->metrics[index];
        hash = fnv1a_str(hash, metric->label);
        hash = fnv1a_str(hash, metric->value_text);
        hash = fnv1a_u8(hash, metric->priority);
    }

    hash = fnv1a_u8(hash, snapshot->capabilities.supports_multiple_windows ? 1u : 0u);
    hash = fnv1a_u8(hash, snapshot->capabilities.supports_manual_refresh ? 1u : 0u);
    hash = fnv1a_u8(hash, snapshot->capabilities.supports_region_label ? 1u : 0u);

    return hash;
}