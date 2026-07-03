#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "unity.h"

#include "app_sleep_schedule.h"

static void test_set_utc_timezone(void)
{
    setenv("TZ", "UTC0", 1);
    tzset();
}

static rtc_time_t test_rtc_time(uint16_t year,
                                uint8_t month,
                                uint8_t day,
                                uint8_t hour,
                                uint8_t minute,
                                uint8_t second)
{
    rtc_time_t rtc_time;

    memset(&rtc_time, 0, sizeof(rtc_time));
    rtc_time.year = year;
    rtc_time.month = month;
    rtc_time.day = day;
    rtc_time.hour = hour;
    rtc_time.minute = minute;
    rtc_time.second = second;
    rtc_time.valid = true;
    return rtc_time;
}

static app_config_t test_schedule_config(bool enabled, uint16_t wake_minutes, uint16_t sleep_minutes)
{
    app_config_t config;

    app_config_init(&config);
    config.device.sleep_schedule.enabled = enabled;
    config.device.sleep_schedule.wake_minutes = wake_minutes;
    config.device.sleep_schedule.sleep_minutes = sleep_minutes;
    return config;
}

TEST_CASE("Sleep schedule treats disabled config as always active", "[sleep][schedule]")
{
    app_config_t config = test_schedule_config(false, APP_CONFIG_TIME_UNSET, APP_CONFIG_TIME_UNSET);
    rtc_time_t rtc_time = test_rtc_time(2026, 6, 30, 2, 15, 0);
    bool active = false;

    test_set_utc_timezone();

    TEST_ASSERT_TRUE(app_sleep_schedule_is_active_now(&config, &rtc_time, &active));
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_FALSE(app_sleep_schedule_seconds_until_transition(&config, &rtc_time, NULL, NULL));
}

TEST_CASE("Sleep schedule handles daytime window with midnight sleep", "[sleep][schedule]")
{
    app_config_t config = test_schedule_config(true, 8 * 60, 24 * 60);
    rtc_time_t before_wake = test_rtc_time(2026, 6, 30, 7, 30, 0);
    rtc_time_t during_active = test_rtc_time(2026, 6, 30, 23, 30, 0);
    bool active = false;
    uint32_t seconds_until_transition = 0;
    bool next_is_sleep = false;

    test_set_utc_timezone();

    TEST_ASSERT_TRUE(app_sleep_schedule_is_active_now(&config, &before_wake, &active));
    TEST_ASSERT_FALSE(active);
    TEST_ASSERT_TRUE(app_sleep_schedule_seconds_until_transition(&config,
                                                                 &before_wake,
                                                                 &seconds_until_transition,
                                                                 &next_is_sleep));
    TEST_ASSERT_FALSE(next_is_sleep);
    TEST_ASSERT_EQUAL_UINT32(30 * 60, seconds_until_transition);

    TEST_ASSERT_TRUE(app_sleep_schedule_is_active_now(&config, &during_active, &active));
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_TRUE(app_sleep_schedule_seconds_until_transition(&config,
                                                                 &during_active,
                                                                 &seconds_until_transition,
                                                                 &next_is_sleep));
    TEST_ASSERT_TRUE(next_is_sleep);
    TEST_ASSERT_EQUAL_UINT32(30 * 60, seconds_until_transition);
}

TEST_CASE("Sleep schedule handles cross-midnight active window", "[sleep][schedule]")
{
    app_config_t config = test_schedule_config(true, 20 * 60, 8 * 60);
    rtc_time_t midday = test_rtc_time(2026, 6, 30, 12, 0, 0);
    rtc_time_t overnight = test_rtc_time(2026, 6, 30, 23, 0, 0);
    rtc_time_t morning = test_rtc_time(2026, 7, 1, 7, 30, 0);
    bool active = false;
    uint32_t seconds_until_transition = 0;
    bool next_is_sleep = false;

    test_set_utc_timezone();

    TEST_ASSERT_TRUE(app_sleep_schedule_is_active_now(&config, &midday, &active));
    TEST_ASSERT_FALSE(active);
    TEST_ASSERT_TRUE(app_sleep_schedule_seconds_until_transition(&config,
                                                                 &midday,
                                                                 &seconds_until_transition,
                                                                 &next_is_sleep));
    TEST_ASSERT_FALSE(next_is_sleep);
    TEST_ASSERT_EQUAL_UINT32(8 * 3600, seconds_until_transition);

    TEST_ASSERT_TRUE(app_sleep_schedule_is_active_now(&config, &overnight, &active));
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_TRUE(app_sleep_schedule_seconds_until_transition(&config,
                                                                 &overnight,
                                                                 &seconds_until_transition,
                                                                 &next_is_sleep));
    TEST_ASSERT_TRUE(next_is_sleep);
    TEST_ASSERT_EQUAL_UINT32(9 * 3600, seconds_until_transition);

    TEST_ASSERT_TRUE(app_sleep_schedule_is_active_now(&config, &morning, &active));
    TEST_ASSERT_TRUE(active);
    TEST_ASSERT_TRUE(app_sleep_schedule_seconds_until_transition(&config,
                                                                 &morning,
                                                                 &seconds_until_transition,
                                                                 &next_is_sleep));
    TEST_ASSERT_TRUE(next_is_sleep);
    TEST_ASSERT_EQUAL_UINT32(30 * 60, seconds_until_transition);
}

TEST_CASE("Sleep schedule can return the next transition wall clock", "[sleep][schedule]")
{
    app_config_t config = test_schedule_config(true, 8 * 60, 23 * 60);
    rtc_time_t rtc_time = test_rtc_time(2026, 6, 30, 22, 55, 0);
    rtc_time_t transition_time;
    bool next_is_sleep = false;

    test_set_utc_timezone();

    TEST_ASSERT_TRUE(app_sleep_schedule_next_transition(&config,
                                                        &rtc_time,
                                                        &transition_time,
                                                        &next_is_sleep));
    TEST_ASSERT_TRUE(next_is_sleep);
    TEST_ASSERT_TRUE(transition_time.valid);
    TEST_ASSERT_EQUAL_UINT8(23, transition_time.hour);
    TEST_ASSERT_EQUAL_UINT8(0, transition_time.minute);
}