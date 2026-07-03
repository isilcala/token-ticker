#ifndef TOKEN_TICKER_MINIMAX_QUOTA_TYPES_H
#define TOKEN_TICKER_MINIMAX_QUOTA_TYPES_H

#include <stddef.h>
#include <stdint.h>

#include "app_config.h"
#include "provider_snapshot.h"

#define MINIMAX_MAX_MODELS 8

typedef struct
{
    char model_name[PROVIDER_SNAPSHOT_LABEL_LEN];
    int64_t start_time_ms;
    int64_t end_time_ms;
    int64_t remains_time_ms;
    int32_t current_interval_total_count;
    int32_t current_interval_usage_count;
    int32_t current_interval_remaining_percent_x100;
    int32_t current_weekly_total_count;
    int32_t current_weekly_usage_count;
    int32_t current_weekly_remaining_percent_x100;
    int32_t current_interval_status;
    int32_t current_weekly_status;
    int64_t weekly_start_time_ms;
    int64_t weekly_end_time_ms;
    int64_t weekly_remains_time_ms;
    int32_t weekly_boost_permille;
} minimax_model_remain_t;

typedef struct
{
    size_t model_count;
    minimax_model_remain_t models[MINIMAX_MAX_MODELS];
} minimax_quota_response_t;

quota_window_status_t minimax_status_to_window_status(int32_t provider_status);
bool minimax_map_quota_response(const provider_config_t *config,
                                const minimax_quota_response_t *response,
                                provider_snapshot_t *snapshot);

#endif