#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "unity.h"

#include "ui_home_view_model.h"

static void test_set_utc_timezone(void)
{
    setenv("TZ", "UTC0", 1);
    tzset();
}

static int64_t test_epoch_from_rtc(const rtc_time_t *rtc_time)
{
    struct tm time_info;

    memset(&time_info, 0, sizeof(time_info));
    time_info.tm_year = (int)rtc_time->year - 1900;
    time_info.tm_mon = (int)rtc_time->month - 1;
    time_info.tm_mday = (int)rtc_time->day;
    time_info.tm_hour = (int)rtc_time->hour;
    time_info.tm_min = (int)rtc_time->minute;
    time_info.tm_sec = (int)rtc_time->second;
    time_info.tm_isdst = -1;

    return (int64_t)mktime(&time_info);
}

static ui_boot_model_t test_boot_model_with_snapshot(void)
{
    ui_boot_model_t boot_model;
    provider_snapshot_t snapshot;
    int64_t now_epoch;

    memset(&boot_model, 0, sizeof(boot_model));
    provider_snapshot_init(&snapshot);

    boot_model.configured = true;
    boot_model.config_result = CONFIG_STORE_LOAD_SD_OVERRIDES_NVS;
    boot_model.sd_config_available = true;
    boot_model.nvs_config_available = true;
    boot_model.wifi_enabled = true;
    boot_model.wifi_has_rssi = true;
    boot_model.wifi_rssi_dbm = -61;

    boot_model.rtc_time.year = 2026;
    boot_model.rtc_time.month = 6;
    boot_model.rtc_time.day = 19;
    boot_model.rtc_time.hour = 12;
    boot_model.rtc_time.minute = 0;
    boot_model.rtc_time.second = 0;
    boot_model.rtc_time.valid = true;
    now_epoch = test_epoch_from_rtc(&boot_model.rtc_time);

    boot_model.provider_last_sync_time.year = 2026;
    boot_model.provider_last_sync_time.month = 6;
    boot_model.provider_last_sync_time.day = 19;
    boot_model.provider_last_sync_time.hour = 11;
    boot_model.provider_last_sync_time.minute = 58;
    boot_model.provider_last_sync_time.second = 0;
    boot_model.provider_last_sync_time.valid = true;
    boot_model.has_provider_last_sync_time = true;

    boot_model.provider_next_attempt_time.year = 2026;
    boot_model.provider_next_attempt_time.month = 6;
    boot_model.provider_next_attempt_time.day = 19;
    boot_model.provider_next_attempt_time.hour = 12;
    boot_model.provider_next_attempt_time.minute = 15;
    boot_model.provider_next_attempt_time.second = 0;
    boot_model.provider_next_attempt_time.valid = true;
    boot_model.has_provider_next_attempt_time = true;

    boot_model.environment.temperature_c_x10 = 253;
    boot_model.environment.humidity_rh_x10 = 601;
    boot_model.environment.valid = true;

    boot_model.power.battery_mv = 3800;
    boot_model.power.battery_percent = 62;
    boot_model.power.charge_state = POWER_CHARGE_STATE_DISCHARGING;
    boot_model.power.valid = true;

    strncpy(snapshot.provider_id, "minimax-cn", sizeof(snapshot.provider_id) - 1);
    strncpy(snapshot.display_name, "MiniMax CN", sizeof(snapshot.display_name) - 1);
    strncpy(snapshot.region, "cn", sizeof(snapshot.region) - 1);
    snapshot.provider_type = PROVIDER_TYPE_MINIMAX;
    snapshot.sync_state = PROVIDER_SYNC_STATE_OK;
    snapshot.window_count = 3;

    strncpy(snapshot.windows[0].id, "general_5h", sizeof(snapshot.windows[0].id) - 1);
    strncpy(snapshot.windows[0].label, "5h quota", sizeof(snapshot.windows[0].label) - 1);
    snapshot.windows[0].used = 33;
    snapshot.windows[0].limit = 100;
    snapshot.windows[0].remaining = 67;
    snapshot.windows[0].remaining_percent_x100 = 6725;
    snapshot.windows[0].start_epoch = now_epoch - (2 * 3600);
    snapshot.windows[0].reset_epoch = now_epoch + (4 * 3600);
    snapshot.windows[0].status = QUOTA_WINDOW_STATUS_LIMITED;

    strncpy(snapshot.windows[1].id, "general_week", sizeof(snapshot.windows[1].id) - 1);
    strncpy(snapshot.windows[1].label, "weekly quota", sizeof(snapshot.windows[1].label) - 1);
    snapshot.windows[1].used = 42;
    snapshot.windows[1].limit = 100;
    snapshot.windows[1].remaining = 58;
    snapshot.windows[1].remaining_percent_x100 = 5825;
    snapshot.windows[1].start_epoch = now_epoch - (24 * 3600);
    snapshot.windows[1].reset_epoch = now_epoch + (2 * 24 * 3600);
    snapshot.windows[1].status = QUOTA_WINDOW_STATUS_LIMITED;

    strncpy(snapshot.windows[2].id, "video_day", sizeof(snapshot.windows[2].id) - 1);
    strncpy(snapshot.windows[2].label, "video/day", sizeof(snapshot.windows[2].label) - 1);
    snapshot.windows[2].used = 3;
    snapshot.windows[2].limit = 8;
    snapshot.windows[2].remaining = 5;
    snapshot.windows[2].remaining_percent_x100 = 6250;
    snapshot.windows[2].start_epoch = now_epoch - (10 * 3600);
    snapshot.windows[2].reset_epoch = now_epoch + (14 * 3600);
    snapshot.windows[2].status = QUOTA_WINDOW_STATUS_LIMITED;

    boot_model.provider_snapshot = snapshot;
    boot_model.has_provider_snapshot = true;
    return boot_model;
}

TEST_CASE("Home view-model formats quota rows and markers", "[ui][view-model]")
{
    ui_boot_model_t boot_model;
    ui_home_view_model_t home_model;

    test_set_utc_timezone();
    boot_model = test_boot_model_with_snapshot();

    ui_home_view_model_build(&boot_model, &home_model);

    TEST_ASSERT_TRUE(home_model.configured);
    TEST_ASSERT_EQUAL_STRING("sd_overrides_nvs sd=yes nvs=yes", home_model.config_text);
    TEST_ASSERT_EQUAL_STRING("SD>NVS", home_model.config_badge_text);

    TEST_ASSERT_TRUE(home_model.has_time);
    TEST_ASSERT_EQUAL_STRING("12:00", home_model.time_text);
    TEST_ASSERT_EQUAL_STRING("06-19", home_model.date_text);
    TEST_ASSERT_EQUAL_STRING("FRI", home_model.weekday_text);

    TEST_ASSERT_TRUE(home_model.has_environment);
    TEST_ASSERT_EQUAL_STRING("25C 60%", home_model.environment_text);

    TEST_ASSERT_TRUE(home_model.has_battery);
    TEST_ASSERT_EQUAL_STRING("3800mV 62%", home_model.battery_text);
    TEST_ASSERT_EQUAL_STRING("62%", home_model.battery_badge_text);

    TEST_ASSERT_TRUE(home_model.has_wifi);
    TEST_ASSERT_TRUE(home_model.wifi_has_signal);
    TEST_ASSERT_EQUAL_UINT8(3, home_model.wifi_signal_bars);

    TEST_ASSERT_TRUE(home_model.has_provider);
    TEST_ASSERT_EQUAL_STRING("MiniMax CN", home_model.provider_name_text);
    TEST_ASSERT_EQUAL_STRING("ok", home_model.provider_sync_text);
    TEST_ASSERT_TRUE(home_model.has_status_capsule);
    TEST_ASSERT_EQUAL_STRING("ok", home_model.status_capsule_text);
    TEST_ASSERT_TRUE(home_model.has_provider_status);
    TEST_ASSERT_TRUE(home_model.provider_status_ok);
    TEST_ASSERT_EQUAL_STRING("Last OK", home_model.provider_status_text);
    TEST_ASSERT_TRUE(home_model.has_provider_last_sync);
    TEST_ASSERT_EQUAL_STRING("11:58", home_model.provider_last_sync_text);
    TEST_ASSERT_TRUE(home_model.has_provider_next_attempt);
    TEST_ASSERT_EQUAL_STRING("12:15", home_model.provider_next_attempt_text);

    TEST_ASSERT_EQUAL_UINT32(3, (uint32_t)home_model.quota_bar_count);

    TEST_ASSERT_EQUAL_STRING("5h quota", home_model.quota_bar_label[0]);
    TEST_ASSERT_EQUAL_STRING("used 33%  R 4h", home_model.quota_bar_value_text[0]);
    TEST_ASSERT_EQUAL_STRING("33%", home_model.quota_bar_percent_text[0]);
    TEST_ASSERT_EQUAL_STRING("Time 33%", home_model.quota_bar_detail_left_text[0]);
    TEST_ASSERT_EQUAL_STRING("R:4h", home_model.quota_bar_detail_right_text[0]);
    TEST_ASSERT_EQUAL_UINT16(3275, home_model.quota_bar_used_x100[0]);
    TEST_ASSERT_TRUE(home_model.quota_bar_has_time_marker[0]);
    TEST_ASSERT_EQUAL_UINT16(3333, home_model.quota_bar_time_x100[0]);
    TEST_ASSERT_FALSE(home_model.quota_bar_segmented[0]);

    TEST_ASSERT_EQUAL_STRING("weekly quota", home_model.quota_bar_label[1]);
    TEST_ASSERT_EQUAL_STRING("used 42%  R 2d", home_model.quota_bar_value_text[1]);
    TEST_ASSERT_EQUAL_STRING("42%", home_model.quota_bar_percent_text[1]);
    TEST_ASSERT_EQUAL_STRING("Time 33%", home_model.quota_bar_detail_left_text[1]);
    TEST_ASSERT_EQUAL_STRING("R:2d", home_model.quota_bar_detail_right_text[1]);
    TEST_ASSERT_EQUAL_UINT16(4175, home_model.quota_bar_used_x100[1]);
    TEST_ASSERT_TRUE(home_model.quota_bar_has_time_marker[1]);
    TEST_ASSERT_EQUAL_UINT16(3333, home_model.quota_bar_time_x100[1]);

    TEST_ASSERT_EQUAL_STRING("video/day", home_model.quota_bar_label[2]);
    TEST_ASSERT_EQUAL_STRING("used 3/8  R 14h", home_model.quota_bar_value_text[2]);
    TEST_ASSERT_EQUAL_STRING("3/8", home_model.quota_bar_percent_text[2]);
    TEST_ASSERT_EQUAL_STRING("5 gen left", home_model.quota_bar_detail_left_text[2]);
    TEST_ASSERT_EQUAL_STRING("R:14h", home_model.quota_bar_detail_right_text[2]);
    TEST_ASSERT_TRUE(home_model.quota_bar_segmented[2]);
    TEST_ASSERT_EQUAL_UINT8(8, home_model.quota_bar_segment_total[2]);
    TEST_ASSERT_EQUAL_UINT8(3, home_model.quota_bar_segment_used[2]);
    TEST_ASSERT_FALSE(home_model.quota_bar_has_time_marker[2]);
}

TEST_CASE("Home view-model keeps battery capsule without provider", "[ui][view-model]")
{
    ui_boot_model_t boot_model;
    ui_home_view_model_t home_model;

    memset(&boot_model, 0, sizeof(boot_model));
    boot_model.config_result = CONFIG_STORE_LOAD_FROM_NVS;
    boot_model.nvs_config_available = true;
    boot_model.power.battery_mv = 3920;
    boot_model.power.battery_percent = 77;
    boot_model.power.valid = true;

    ui_home_view_model_build(&boot_model, &home_model);

    TEST_ASSERT_EQUAL_STRING("nvs sd=no nvs=yes", home_model.config_text);
    TEST_ASSERT_EQUAL_STRING("NVS", home_model.config_badge_text);
    TEST_ASSERT_TRUE(home_model.has_status_capsule);
    TEST_ASSERT_EQUAL_STRING("BAT 77%", home_model.status_capsule_text);
    TEST_ASSERT_FALSE(home_model.has_provider);
    TEST_ASSERT_EQUAL_UINT32(0, (uint32_t)home_model.quota_bar_count);
}