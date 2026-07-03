#include <stdbool.h>
#include <string.h>

#include "unity.h"

#include "config_store.h"

static const char *VALID_WIFI_JSON =
    "{"
    "\"version\":1,"
    "\"wifi\":{\"enabled\":true,\"ssid\":\"Elvin_IoT\",\"password\":\"secret\"},"
    "\"display\":{\"active_provider_id\":\"minimax-cn\",\"weather_enabled\":false},"
    "\"providers\":[{\"id\":\"minimax-cn\",\"type\":\"minimax\",\"enabled\":true,\"region\":\"cn\",\"api_key\":\"key-1\"}]"
    "}";

static const char *IMPLICIT_WIFI_ENABLE_JSON =
    "{"
    "\"version\":1,"
    "\"wifi\":{\"ssid\":\"Elvin_IoT\",\"password\":\"secret\"},"
    "\"display\":{\"active_provider_id\":\"minimax-cn\",\"weather_enabled\":false},"
    "\"providers\":[{\"id\":\"minimax-cn\",\"type\":\"minimax\",\"enabled\":true,\"region\":\"cn\",\"api_key\":\"key-1\"}]"
    "}";

static const char *MISSING_WIFI_SSID_JSON =
    "{"
    "\"version\":1,"
    "\"wifi\":{\"enabled\":true,\"password\":\"secret\"},"
    "\"display\":{\"active_provider_id\":\"minimax-cn\",\"weather_enabled\":false},"
    "\"providers\":[{\"id\":\"minimax-cn\",\"type\":\"minimax\",\"enabled\":true,\"region\":\"cn\",\"api_key\":\"key-1\"}]"
    "}";

static const char *VALID_SLEEP_SCHEDULE_JSON =
    "{"
    "\"version\":1,"
    "\"device\":{"
    "\"timezone\":\"Asia/Shanghai\","
    "\"sleep_schedule\":{\"enabled\":true,\"wake_time\":\"08:00\",\"sleep_time\":\"24:00\",\"manual_wake_minutes\":7}"
    "},"
    "\"display\":{\"active_provider_id\":\"minimax-cn\",\"weather_enabled\":false},"
    "\"providers\":[{\"id\":\"minimax-cn\",\"type\":\"minimax\",\"enabled\":true,\"region\":\"cn\",\"api_key\":\"key-1\"}]"
    "}";

static const char *CROSS_MIDNIGHT_SLEEP_SCHEDULE_JSON =
    "{"
    "\"version\":1,"
    "\"device\":{"
    "\"sleep_schedule\":{\"enabled\":true,\"wake_time\":\"20:00\",\"sleep_time\":\"08:00\"}"
    "},"
    "\"display\":{\"active_provider_id\":\"minimax-cn\",\"weather_enabled\":false},"
    "\"providers\":[{\"id\":\"minimax-cn\",\"type\":\"minimax\",\"enabled\":true,\"region\":\"cn\",\"api_key\":\"key-1\"}]"
    "}";

static const char *INVALID_SLEEP_SCHEDULE_JSON =
    "{"
    "\"version\":1,"
    "\"device\":{"
    "\"sleep_schedule\":{\"enabled\":true,\"wake_time\":\"00:00\",\"sleep_time\":\"24:00\"}"
    "},"
    "\"display\":{\"active_provider_id\":\"minimax-cn\",\"weather_enabled\":false},"
    "\"providers\":[{\"id\":\"minimax-cn\",\"type\":\"minimax\",\"enabled\":true,\"region\":\"cn\",\"api_key\":\"key-1\"}]"
    "}";

static const char *INVALID_MANUAL_WAKE_MINUTES_JSON =
    "{"
    "\"version\":1,"
    "\"device\":{"
    "\"sleep_schedule\":{\"enabled\":true,\"wake_time\":\"08:00\",\"sleep_time\":\"24:00\",\"manual_wake_minutes\":-1}"
    "},"
    "\"display\":{\"active_provider_id\":\"minimax-cn\",\"weather_enabled\":false},"
    "\"providers\":[{\"id\":\"minimax-cn\",\"type\":\"minimax\",\"enabled\":true,\"region\":\"cn\",\"api_key\":\"key-1\"}]"
    "}";

static const char *LATCHED_MANUAL_WAKE_MINUTES_JSON =
    "{"
    "\"version\":1,"
    "\"device\":{"
    "\"sleep_schedule\":{\"enabled\":true,\"wake_time\":\"08:00\",\"sleep_time\":\"24:00\",\"manual_wake_minutes\":0}"
    "},"
    "\"display\":{\"active_provider_id\":\"minimax-cn\",\"weather_enabled\":false},"
    "\"providers\":[{\"id\":\"minimax-cn\",\"type\":\"minimax\",\"enabled\":true,\"region\":\"cn\",\"api_key\":\"key-1\"}]"
    "}";

TEST_CASE("Config parser keeps Wi-Fi credentials and active provider", "[config][wifi]")
{
    app_config_t config;

    TEST_ASSERT_TRUE(config_store_parse_json_text(VALID_WIFI_JSON, &config));
    TEST_ASSERT_TRUE(config.wifi.enabled);
    TEST_ASSERT_EQUAL_STRING("Elvin_IoT", config.wifi.ssid);
    TEST_ASSERT_EQUAL_STRING("secret", config.wifi.password);
    TEST_ASSERT_EQUAL_STRING("minimax-cn", config.display.active_provider_id);
    TEST_ASSERT_EQUAL_UINT32(1, (uint32_t)config.provider_count);
    TEST_ASSERT_TRUE(config.providers[0].enabled);
    TEST_ASSERT_EQUAL(PROVIDER_TYPE_MINIMAX, config.providers[0].provider_type);
}

TEST_CASE("Config parser auto-enables Wi-Fi when SSID is present", "[config][wifi]")
{
    app_config_t config;

    TEST_ASSERT_TRUE(config_store_parse_json_text(IMPLICIT_WIFI_ENABLE_JSON, &config));
    TEST_ASSERT_TRUE(config.wifi.enabled);
    TEST_ASSERT_EQUAL_STRING("Elvin_IoT", config.wifi.ssid);
}

TEST_CASE("Config validation rejects enabled Wi-Fi without SSID", "[config][wifi]")
{
    app_config_t config;

    TEST_ASSERT_FALSE(config_store_parse_json_text(MISSING_WIFI_SSID_JSON, &config));
}

TEST_CASE("Config parser keeps sleep schedule wall-clock times", "[config][sleep]")
{
    app_config_t config;

    TEST_ASSERT_TRUE(config_store_parse_json_text(VALID_SLEEP_SCHEDULE_JSON, &config));
    TEST_ASSERT_TRUE(config.device.sleep_schedule.enabled);
    TEST_ASSERT_EQUAL_UINT16(8 * 60, config.device.sleep_schedule.wake_minutes);
    TEST_ASSERT_EQUAL_UINT16(24 * 60, config.device.sleep_schedule.sleep_minutes);
    TEST_ASSERT_EQUAL_UINT16(7, config.device.sleep_schedule.manual_wake_minutes);
}

TEST_CASE("Config parser accepts cross-midnight sleep schedule", "[config][sleep]")
{
    app_config_t config;

    TEST_ASSERT_TRUE(config_store_parse_json_text(CROSS_MIDNIGHT_SLEEP_SCHEDULE_JSON, &config));
    TEST_ASSERT_TRUE(config.device.sleep_schedule.enabled);
    TEST_ASSERT_EQUAL_UINT16(20 * 60, config.device.sleep_schedule.wake_minutes);
    TEST_ASSERT_EQUAL_UINT16(8 * 60, config.device.sleep_schedule.sleep_minutes);
}

TEST_CASE("Config validation rejects equivalent wake and sleep times", "[config][sleep]")
{
    app_config_t config;

    TEST_ASSERT_FALSE(config_store_parse_json_text(INVALID_SLEEP_SCHEDULE_JSON, &config));
}

TEST_CASE("Config validation rejects invalid manual wake minutes", "[config][sleep]")
{
    app_config_t config;

    TEST_ASSERT_FALSE(config_store_parse_json_text(INVALID_MANUAL_WAKE_MINUTES_JSON, &config));
}

TEST_CASE("Config parser allows latched manual wake minutes", "[config][sleep]")
{
    app_config_t config;

    TEST_ASSERT_TRUE(config_store_parse_json_text(LATCHED_MANUAL_WAKE_MINUTES_JSON, &config));
    TEST_ASSERT_TRUE(config.device.sleep_schedule.enabled);
    TEST_ASSERT_EQUAL_UINT16(0, config.device.sleep_schedule.manual_wake_minutes);
}

TEST_CASE("Config precedence prefers valid SD over missing or stale NVS", "[config][precedence]")
{
    bool persist_sd_to_nvs = false;

    TEST_ASSERT_EQUAL(CONFIG_STORE_LOAD_FROM_SD,
                      config_store_resolve_precedence(true,
                                                      VALID_WIFI_JSON,
                                                      false,
                                                      NULL,
                                                      &persist_sd_to_nvs));
    TEST_ASSERT_TRUE(persist_sd_to_nvs);

    persist_sd_to_nvs = false;
    TEST_ASSERT_EQUAL(CONFIG_STORE_LOAD_SD_OVERRIDES_NVS,
                      config_store_resolve_precedence(true,
                                                      VALID_WIFI_JSON,
                                                      true,
                                                      IMPLICIT_WIFI_ENABLE_JSON,
                                                      &persist_sd_to_nvs));
    TEST_ASSERT_TRUE(persist_sd_to_nvs);

    persist_sd_to_nvs = true;
    TEST_ASSERT_EQUAL(CONFIG_STORE_LOAD_FROM_SD,
                      config_store_resolve_precedence(true,
                                                      VALID_WIFI_JSON,
                                                      true,
                                                      VALID_WIFI_JSON,
                                                      &persist_sd_to_nvs));
    TEST_ASSERT_FALSE(persist_sd_to_nvs);
}

TEST_CASE("Config precedence falls back to NVS or unprovisioned", "[config][precedence]")
{
    bool persist_sd_to_nvs = true;

    TEST_ASSERT_EQUAL(CONFIG_STORE_LOAD_FROM_NVS,
                      config_store_resolve_precedence(false,
                                                      NULL,
                                                      true,
                                                      VALID_WIFI_JSON,
                                                      &persist_sd_to_nvs));
    TEST_ASSERT_FALSE(persist_sd_to_nvs);

    TEST_ASSERT_EQUAL(CONFIG_STORE_LOAD_UNPROVISIONED,
                      config_store_resolve_precedence(false,
                                                      NULL,
                                                      false,
                                                      NULL,
                                                      &persist_sd_to_nvs));
    TEST_ASSERT_FALSE(persist_sd_to_nvs);
}