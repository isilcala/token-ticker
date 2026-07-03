#ifndef TOKEN_TICKER_UI_HOME_VIEW_MODEL_H
#define TOKEN_TICKER_UI_HOME_VIEW_MODEL_H

#include <stdbool.h>
#include <stddef.h>

#include "ui_boot_model.h"

#define UI_HOME_TEXT_LEN 64
#define UI_HOME_MAX_QUOTA_BARS 3
#define UI_HOME_MAX_QUOTA_SEGMENTS 8

typedef struct
{
    bool configured;
    char config_text[UI_HOME_TEXT_LEN];
    char config_badge_text[UI_HOME_TEXT_LEN];

    bool has_time;
    char time_text[UI_HOME_TEXT_LEN];
    char date_text[UI_HOME_TEXT_LEN];
    char weekday_text[UI_HOME_TEXT_LEN];

    bool has_environment;
    char environment_text[UI_HOME_TEXT_LEN];

    bool has_battery;
    char battery_text[UI_HOME_TEXT_LEN];
    char battery_badge_text[UI_HOME_TEXT_LEN];

    bool has_wifi;
    bool wifi_connected;
    bool wifi_has_signal;
    uint8_t wifi_signal_bars;

    bool has_status_capsule;
    char status_capsule_text[UI_HOME_TEXT_LEN];

    bool has_provider;
    char provider_name_text[UI_HOME_TEXT_LEN];
    char provider_sync_text[UI_HOME_TEXT_LEN];
    bool has_provider_status;
    bool provider_status_ok;
    char provider_status_text[UI_HOME_TEXT_LEN];
    bool has_provider_last_sync;
    char provider_last_sync_text[UI_HOME_TEXT_LEN];
    bool has_provider_next_attempt;
    char provider_next_attempt_text[UI_HOME_TEXT_LEN];
    size_t quota_bar_count;
    char quota_bar_label[UI_HOME_MAX_QUOTA_BARS][UI_HOME_TEXT_LEN];
    char quota_bar_value_text[UI_HOME_MAX_QUOTA_BARS][UI_HOME_TEXT_LEN];
    char quota_bar_amount_text[UI_HOME_MAX_QUOTA_BARS][UI_HOME_TEXT_LEN];
    char quota_bar_percent_text[UI_HOME_MAX_QUOTA_BARS][UI_HOME_TEXT_LEN];
    char quota_bar_detail_left_text[UI_HOME_MAX_QUOTA_BARS][UI_HOME_TEXT_LEN];
    char quota_bar_detail_right_text[UI_HOME_MAX_QUOTA_BARS][UI_HOME_TEXT_LEN];
    uint16_t quota_bar_used_x100[UI_HOME_MAX_QUOTA_BARS];
    bool quota_bar_has_time_marker[UI_HOME_MAX_QUOTA_BARS];
    uint16_t quota_bar_time_x100[UI_HOME_MAX_QUOTA_BARS];
    bool quota_bar_segmented[UI_HOME_MAX_QUOTA_BARS];
    uint8_t quota_bar_segment_total[UI_HOME_MAX_QUOTA_BARS];
    uint8_t quota_bar_segment_used[UI_HOME_MAX_QUOTA_BARS];
} ui_home_view_model_t;

void ui_home_view_model_build(const ui_boot_model_t *boot_model, ui_home_view_model_t *home_model);

#endif