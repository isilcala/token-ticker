#include <string.h>

#include "unity.h"

#include "app_scheduler.h"
#include "provider_poll_state.h"
#include "provider_snapshot.h"
#include "time_service.h"

static provider_snapshot_t test_snapshot_with_used(int32_t used)
{
    provider_snapshot_t snapshot;

    provider_snapshot_init(&snapshot);
    strncpy(snapshot.provider_id, "minimax-cn", sizeof(snapshot.provider_id) - 1);
    strncpy(snapshot.display_name, "MiniMax CN", sizeof(snapshot.display_name) - 1);
    strncpy(snapshot.region, "cn", sizeof(snapshot.region) - 1);
    snapshot.provider_type = PROVIDER_TYPE_MINIMAX;
    snapshot.sync_state = PROVIDER_SYNC_STATE_OK;
    snapshot.window_count = 1;
    strncpy(snapshot.windows[0].id, "general_5h", sizeof(snapshot.windows[0].id) - 1);
    strncpy(snapshot.windows[0].label, "5h quota", sizeof(snapshot.windows[0].label) - 1);
    snapshot.windows[0].used = used;
    snapshot.windows[0].limit = 100;
    snapshot.windows[0].remaining = 100 - used;
    snapshot.windows[0].remaining_percent_x100 = (100 - used) * 100;
    snapshot.windows[0].start_epoch = 1776355200LL;
    snapshot.windows[0].reset_epoch = 1776373200LL;
    snapshot.windows[0].status = QUOTA_WINDOW_STATUS_LIMITED;

    return snapshot;
}

TEST_CASE("Scheduler marks first-run work as due", "[scheduler][cadence]")
{
    app_scheduler_state_t scheduler_state;
    app_scheduler_due_t due;
    provider_poll_state_t poll_state;

    app_scheduler_init(&scheduler_state, true);
    provider_poll_state_init(&poll_state);
    time_service_init(true, 24, "Asia/Shanghai");

    app_scheduler_compute_due(&scheduler_state,
                              1700000000LL,
                              &poll_state,
                              time_service_get_state(),
                              &due);

    TEST_ASSERT_TRUE(due.clock_due);
    TEST_ASSERT_TRUE(due.provider_due);
    TEST_ASSERT_TRUE(due.environment_due);
    TEST_ASSERT_TRUE(due.power_due);
    TEST_ASSERT_TRUE(due.weather_due);
    TEST_ASSERT_TRUE(due.ntp_due);
}

TEST_CASE("Scheduler respects normal poll and refresh thresholds", "[scheduler][cadence]")
{
    const int64_t base_epoch = 1700000000LL;
    app_scheduler_state_t scheduler_state;
    app_scheduler_due_t due;
    provider_poll_state_t poll_state;

    app_scheduler_init(&scheduler_state, true);
    provider_poll_state_init(&poll_state);
    time_service_init(true, 24, "Asia/Shanghai");
    time_service_mark_time_valid(base_epoch, base_epoch);

    app_scheduler_note_clock_rendered(&scheduler_state, base_epoch);
    app_scheduler_note_provider_polled(&scheduler_state, base_epoch);
    app_scheduler_note_environment_polled(&scheduler_state, base_epoch);
    app_scheduler_note_power_polled(&scheduler_state, base_epoch);
    app_scheduler_note_weather_polled(&scheduler_state, base_epoch);
    app_scheduler_note_ntp_attempted(&scheduler_state, base_epoch);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 30,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_FALSE(due.clock_due);
    TEST_ASSERT_FALSE(due.provider_due);
    TEST_ASSERT_FALSE(due.environment_due);
    TEST_ASSERT_FALSE(due.power_due);
    TEST_ASSERT_FALSE(due.weather_due);
    TEST_ASSERT_FALSE(due.ntp_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 899,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_FALSE(due.provider_due);
    TEST_ASSERT_FALSE(due.environment_due);
    TEST_ASSERT_FALSE(due.power_due);
    TEST_ASSERT_FALSE(due.weather_due);
    TEST_ASSERT_FALSE(due.ntp_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 900,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.provider_due);
    TEST_ASSERT_TRUE(due.environment_due);
    TEST_ASSERT_FALSE(due.power_due);
    TEST_ASSERT_FALSE(due.weather_due);
    TEST_ASSERT_FALSE(due.ntp_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 1800,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.power_due);
    TEST_ASSERT_FALSE(due.weather_due);
    TEST_ASSERT_FALSE(due.ntp_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 3600,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.weather_due);
    TEST_ASSERT_FALSE(due.ntp_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + (24 * 3600),
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.ntp_due);
}

TEST_CASE("Scheduler follows provider high and low bands", "[scheduler][cadence]")
{
    const int64_t base_epoch = 1700000000LL;
    app_scheduler_state_t scheduler_state;
    app_scheduler_due_t due;
    provider_poll_state_t poll_state;
    provider_snapshot_t snapshot_a = test_snapshot_with_used(10);
    provider_snapshot_t snapshot_b = test_snapshot_with_used(11);
    bool changed = false;

    app_scheduler_init(&scheduler_state, false);
    app_scheduler_note_provider_polled(&scheduler_state, base_epoch);
    time_service_init(false, 24, "Asia/Shanghai");

    provider_poll_state_init(&poll_state);
    TEST_ASSERT_TRUE(provider_poll_state_note_success(&poll_state, &snapshot_a, false, &changed));
    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_TRUE(provider_poll_state_note_success(&poll_state, &snapshot_b, false, &changed));
    TEST_ASSERT_TRUE(changed);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 299,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_FALSE(due.provider_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 300,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.provider_due);

    provider_poll_state_init(&poll_state);
    TEST_ASSERT_TRUE(provider_poll_state_note_success(&poll_state, &snapshot_a, false, &changed));
    TEST_ASSERT_TRUE(provider_poll_state_note_success(&poll_state, &snapshot_a, false, &changed));
    TEST_ASSERT_FALSE(changed);
    TEST_ASSERT_TRUE(provider_poll_state_note_success(&poll_state, &snapshot_a, false, &changed));
    TEST_ASSERT_FALSE(changed);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 1799,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_FALSE(due.provider_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 1800,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.provider_due);
}

TEST_CASE("Scheduler applies provider failure backoff", "[scheduler][cadence]")
{
    const int64_t base_epoch = 1700000000LL;
    app_scheduler_state_t scheduler_state;
    app_scheduler_due_t due;
    provider_poll_state_t poll_state;

    app_scheduler_init(&scheduler_state, false);
    app_scheduler_note_provider_polled(&scheduler_state, base_epoch);
    provider_poll_state_init(&poll_state);
    time_service_init(false, 24, "Asia/Shanghai");

    provider_poll_state_note_failure(&poll_state);
    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 599,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_FALSE(due.provider_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 600,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.provider_due);

    provider_poll_state_note_failure(&poll_state);
    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 1799,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_FALSE(due.provider_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 1800,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.provider_due);

    provider_poll_state_note_failure(&poll_state);
    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 3599,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_FALSE(due.provider_due);

    app_scheduler_compute_due(&scheduler_state,
                              base_epoch + 3600,
                              &poll_state,
                              time_service_get_state(),
                              &due);
    TEST_ASSERT_TRUE(due.provider_due);
}