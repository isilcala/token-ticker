#include <stdbool.h>
#include <string.h>

#include "unity.h"

#include "app_config.h"
#include "minimax_quota_types.h"
#include "provider_snapshot.h"

bool minimax_http_parse_quota_response_json(const char *json_text, minimax_quota_response_t *response);

extern const char minimax_cn_general_video_quota_json_start[] asm("_binary_minimax_cn_general_video_quota_json_start");

static provider_config_t test_minimax_cn_config(void)
{
    provider_config_t config;

    memset(&config, 0, sizeof(config));
    strncpy(config.id, "minimax-cn", sizeof(config.id) - 1);
    config.provider_type = PROVIDER_TYPE_MINIMAX;
    config.enabled = true;
    strncpy(config.region, "cn", sizeof(config.region) - 1);
    strncpy(config.api_key, "test-api-key", sizeof(config.api_key) - 1);

    return config;
}

TEST_CASE("MiniMax parser accepts percentage and count windows", "[minimax][parser]")
{
    minimax_quota_response_t response;

    TEST_ASSERT_TRUE(minimax_http_parse_quota_response_json(minimax_cn_general_video_quota_json_start, &response));
    TEST_ASSERT_EQUAL_UINT32(2, (uint32_t)response.model_count);

    TEST_ASSERT_EQUAL_STRING("general", response.models[0].model_name);
    TEST_ASSERT_EQUAL_INT64(1776355200000LL, response.models[0].start_time_ms);
    TEST_ASSERT_EQUAL_INT32(6725, response.models[0].current_interval_remaining_percent_x100);
    TEST_ASSERT_EQUAL_INT32(5825, response.models[0].current_weekly_remaining_percent_x100);
    TEST_ASSERT_EQUAL_INT32(1250, response.models[0].weekly_boost_permille);

    TEST_ASSERT_EQUAL_STRING("video", response.models[1].model_name);
    TEST_ASSERT_EQUAL_INT32(8, response.models[1].current_interval_total_count);
    TEST_ASSERT_EQUAL_INT32(3, response.models[1].current_interval_usage_count);
    TEST_ASSERT_EQUAL_INT32(1000, response.models[1].weekly_boost_permille);
}

TEST_CASE("MiniMax mapper normalizes general and video windows", "[minimax][mapper]")
{
    minimax_quota_response_t response;
    provider_snapshot_t snapshot;
    provider_config_t config = test_minimax_cn_config();

    TEST_ASSERT_TRUE(minimax_http_parse_quota_response_json(minimax_cn_general_video_quota_json_start, &response));
    TEST_ASSERT_TRUE(minimax_map_quota_response(&config, &response, &snapshot));

    TEST_ASSERT_EQUAL(PROVIDER_TYPE_MINIMAX, snapshot.provider_type);
    TEST_ASSERT_EQUAL(PROVIDER_SYNC_STATE_OK, snapshot.sync_state);
    TEST_ASSERT_FALSE(snapshot.stale);
    TEST_ASSERT_EQUAL_STRING("minimax-cn", snapshot.provider_id);
    TEST_ASSERT_EQUAL_STRING("MiniMax CN", snapshot.display_name);
    TEST_ASSERT_EQUAL_STRING("cn", snapshot.region);
    TEST_ASSERT_EQUAL_UINT32(3, (uint32_t)snapshot.window_count);
    TEST_ASSERT_EQUAL_UINT32(2, (uint32_t)snapshot.metric_count);
    TEST_ASSERT_EQUAL_INT64(1776373200LL, snapshot.last_success_epoch);

    TEST_ASSERT_EQUAL_STRING("general_5h", snapshot.windows[0].id);
    TEST_ASSERT_EQUAL_STRING("5h quota", snapshot.windows[0].label);
    TEST_ASSERT_EQUAL_INT32(33, snapshot.windows[0].used);
    TEST_ASSERT_EQUAL_INT32(100, snapshot.windows[0].limit);
    TEST_ASSERT_EQUAL_INT32(67, snapshot.windows[0].remaining);
    TEST_ASSERT_EQUAL_INT32(6725, snapshot.windows[0].remaining_percent_x100);
    TEST_ASSERT_EQUAL_INT64(1776355200LL, snapshot.windows[0].start_epoch);
    TEST_ASSERT_EQUAL_INT64(1776373200LL, snapshot.windows[0].reset_epoch);
    TEST_ASSERT_EQUAL(QUOTA_WINDOW_STATUS_LIMITED, snapshot.windows[0].status);

    TEST_ASSERT_EQUAL_STRING("general_week", snapshot.windows[1].id);
    TEST_ASSERT_EQUAL_STRING("weekly quota", snapshot.windows[1].label);
    TEST_ASSERT_EQUAL_INT32(42, snapshot.windows[1].used);
    TEST_ASSERT_EQUAL_INT32(100, snapshot.windows[1].limit);
    TEST_ASSERT_EQUAL_INT32(58, snapshot.windows[1].remaining);
    TEST_ASSERT_EQUAL_INT32(5825, snapshot.windows[1].remaining_percent_x100);
    TEST_ASSERT_EQUAL_INT64(1776297600LL, snapshot.windows[1].start_epoch);
    TEST_ASSERT_EQUAL_INT64(1776902400LL, snapshot.windows[1].reset_epoch);

    TEST_ASSERT_EQUAL_STRING("video_day", snapshot.windows[2].id);
    TEST_ASSERT_EQUAL_STRING("video/day", snapshot.windows[2].label);
    TEST_ASSERT_EQUAL_INT32(3, snapshot.windows[2].used);
    TEST_ASSERT_EQUAL_INT32(8, snapshot.windows[2].limit);
    TEST_ASSERT_EQUAL_INT32(5, snapshot.windows[2].remaining);
    TEST_ASSERT_EQUAL_INT32(6250, snapshot.windows[2].remaining_percent_x100);
    TEST_ASSERT_EQUAL_INT64(1776355200LL, snapshot.windows[2].start_epoch);
    TEST_ASSERT_EQUAL_INT64(1776441600LL, snapshot.windows[2].reset_epoch);
    TEST_ASSERT_EQUAL(QUOTA_WINDOW_STATUS_LIMITED, snapshot.windows[2].status);

    TEST_ASSERT_EQUAL_STRING("models", snapshot.metrics[0].label);
    TEST_ASSERT_EQUAL_STRING("2", snapshot.metrics[0].value_text);
    TEST_ASSERT_EQUAL_STRING("window_count", snapshot.metrics[1].label);
    TEST_ASSERT_EQUAL_STRING("3", snapshot.metrics[1].value_text);
}