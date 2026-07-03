#ifndef TOKEN_TICKER_PROVIDER_SNAPSHOT_H
#define TOKEN_TICKER_PROVIDER_SNAPSHOT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PROVIDER_SNAPSHOT_MAX_WINDOWS 4
#define PROVIDER_SNAPSHOT_MAX_METRICS 6
#define PROVIDER_SNAPSHOT_LABEL_LEN 32
#define PROVIDER_SNAPSHOT_VALUE_LEN 48

typedef enum
{
    PROVIDER_TYPE_UNKNOWN = 0,
    PROVIDER_TYPE_MINIMAX,
} provider_type_t;

typedef enum
{
    PROVIDER_SYNC_STATE_IDLE = 0,
    PROVIDER_SYNC_STATE_OK,
    PROVIDER_SYNC_STATE_AUTH_ERROR,
    PROVIDER_SYNC_STATE_NETWORK_ERROR,
    PROVIDER_SYNC_STATE_PARSE_ERROR,
    PROVIDER_SYNC_STATE_UNSUPPORTED,
} provider_sync_state_t;

typedef enum
{
    QUOTA_WINDOW_STATUS_UNKNOWN = 0,
    QUOTA_WINDOW_STATUS_LIMITED,
    QUOTA_WINDOW_STATUS_EXHAUSTED,
    QUOTA_WINDOW_STATUS_UNLIMITED,
} quota_window_status_t;

typedef struct
{
    char id[PROVIDER_SNAPSHOT_LABEL_LEN];
    char label[PROVIDER_SNAPSHOT_LABEL_LEN];
    int32_t used;
    int32_t limit;
    int32_t remaining;
    int32_t remaining_percent_x100;
    int64_t start_epoch;
    int64_t reset_epoch;
    quota_window_status_t status;
} quota_window_t;

typedef struct
{
    char label[PROVIDER_SNAPSHOT_LABEL_LEN];
    char value_text[PROVIDER_SNAPSHOT_VALUE_LEN];
    uint8_t priority;
} detail_metric_t;

typedef struct
{
    bool supports_multiple_windows;
    bool supports_manual_refresh;
    bool supports_region_label;
} provider_capabilities_t;

typedef struct
{
    char provider_id[PROVIDER_SNAPSHOT_LABEL_LEN];
    char display_name[PROVIDER_SNAPSHOT_LABEL_LEN];
    char region[PROVIDER_SNAPSHOT_LABEL_LEN];
    provider_type_t provider_type;
    provider_sync_state_t sync_state;
    bool stale;
    int64_t last_success_epoch;
    uint8_t primary_window_index;
    size_t window_count;
    quota_window_t windows[PROVIDER_SNAPSHOT_MAX_WINDOWS];
    size_t metric_count;
    detail_metric_t metrics[PROVIDER_SNAPSHOT_MAX_METRICS];
    provider_capabilities_t capabilities;
} provider_snapshot_t;

void provider_snapshot_init(provider_snapshot_t *snapshot);

#endif