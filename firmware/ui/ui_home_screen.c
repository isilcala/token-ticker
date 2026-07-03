#include "ui_home_screen.h"

#include <stdio.h>
#include <string.h>

#include "lvgl.h"

#define UI_HOME_SCREEN_WIDTH 400
#define UI_HOME_SCREEN_HEIGHT 300
#define UI_HOME_STATUS_BAR_HEIGHT 36
#define UI_HOME_CONTENT_HEIGHT (UI_HOME_SCREEN_HEIGHT - UI_HOME_STATUS_BAR_HEIGHT)
#define UI_HOME_SIDE_PADDING 10
#define UI_HOME_BAR_WIDTH 380
#define UI_HOME_BAR_X UI_HOME_SIDE_PADDING
#define UI_HOME_BAR_HEIGHT 22
#define UI_HOME_SEGMENT_BAR_HEIGHT 22
#define UI_HOME_BOTTOM_ROW_Y 250
#define UI_HOME_ROW_BAR_Y_OFFSET 24
#define UI_HOME_ROW_MARKER_HEAD_Y_OFFSET 20
#define UI_HOME_ROW_MARKER_STEM_Y_OFFSET 23
#define UI_HOME_ROW_DETAIL_Y_OFFSET 50
#define UI_HOME_ROW_DIVIDER_Y_OFFSET 68

static const lv_font_t *TIME_FONT = &lv_font_montserrat_24;
static const lv_font_t *TEXT_FONT = &lv_font_montserrat_12;
static const lv_font_t *STATUS_FONT = &lv_font_montserrat_14;
static const lv_font_t *LABEL_FONT = &lv_font_montserrat_16;
static const lv_coord_t QUOTA_ROW_BASE_Y[UI_HOME_MAX_QUOTA_BARS] = {18, 92, 166};
static const lv_coord_t QUOTA_LABEL_WIDTH = 228;
static const lv_coord_t QUOTA_AMOUNT_X = 244;
static const lv_coord_t QUOTA_AMOUNT_WIDTH = 82;
static const lv_coord_t QUOTA_PERCENT_X = 332;
static const lv_coord_t QUOTA_PERCENT_WIDTH = 48;
static const uint16_t QUOTA_TICK_PCT[3] = {2500, 5000, 7500};

typedef struct
{
    lv_obj_t *label;
    lv_obj_t *amount;
    lv_obj_t *percent;
    lv_obj_t *bar;
    lv_obj_t *tick[3];
    lv_obj_t *marker_stem;
    lv_obj_t *marker_head[3];
    lv_obj_t *detail_left;
    lv_obj_t *detail_right;
    lv_obj_t *segment[UI_HOME_MAX_QUOTA_SEGMENTS];
} ui_home_quota_row_t;

typedef struct
{
    bool initialized;
    lv_obj_t *screen;
    lv_obj_t *status_bar;
    lv_obj_t *content;
    lv_obj_t *time_label;
    lv_obj_t *date_label;
    lv_obj_t *weekday_label;
    lv_obj_t *provider_badge;
    lv_obj_t *provider_badge_label;
    lv_obj_t *environment_label;
    lv_obj_t *battery_body;
    lv_obj_t *battery_segment[4];
    lv_obj_t *battery_tip;
    lv_obj_t *battery_percent_label;
    lv_obj_t *wifi_bar[3];
    ui_home_quota_row_t quota_row[UI_HOME_MAX_QUOTA_BARS];
    lv_obj_t *divider[3];
    lv_obj_t *empty_label;
    lv_obj_t *bottom_divider;
    lv_obj_t *bottom_status_indicator;
    lv_obj_t *bottom_status_label;
    lv_obj_t *bottom_synced_label;
    lv_obj_t *bottom_next_label;
} ui_home_screen_state_t;

static ui_home_screen_state_t s_home;

static lv_obj_t *ui_home_create_rect(lv_obj_t *parent,
                                     lv_coord_t x,
                                     lv_coord_t y,
                                     lv_coord_t w,
                                     lv_coord_t h,
                                     lv_color_t bg_color,
                                     lv_opa_t bg_opa,
                                     lv_color_t border_color,
                                     lv_coord_t border_width)
{
    lv_obj_t *obj = lv_obj_create(parent);

    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, bg_color, 0);
    lv_obj_set_style_bg_opa(obj, bg_opa, 0);
    lv_obj_set_style_border_color(obj, border_color, 0);
    lv_obj_set_style_border_width(obj, border_width, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    return obj;
}

static lv_obj_t *ui_home_create_label(lv_obj_t *parent,
                                      const lv_font_t *font,
                                      lv_color_t text_color,
                                      lv_coord_t x,
                                      lv_coord_t y,
                                      lv_coord_t w,
                                      lv_text_align_t align)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_obj_set_pos(label, x, y);
    if (w > 0)
    {
        lv_obj_set_width(label, w);
    }
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, text_color, 0);
    lv_obj_set_style_text_align(label, align, 0);
    lv_obj_set_style_pad_all(label, 0, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_label_set_text(label, "");
    return label;
}

static lv_obj_t *ui_home_create_quota_bar(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{
    lv_obj_t *bar = lv_bar_create(parent);

    lv_obj_set_pos(bar, x, y);
    lv_obj_set_size(bar, UI_HOME_BAR_WIDTH, UI_HOME_BAR_HEIGHT);
    lv_bar_set_range(bar, 0, 10000);
    lv_obj_set_style_radius(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 0, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(bar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(bar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_black(), LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(bar, 0, LV_PART_MAIN);
    return bar;
}

static void ui_home_set_hidden(lv_obj_t *obj, bool hidden)
{
    if (obj == NULL)
    {
        return;
    }

    if (hidden)
    {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

static uint8_t ui_home_battery_segments_filled(const char *battery_badge_text)
{
    unsigned percent = 0;

    if (battery_badge_text == NULL || sscanf(battery_badge_text, "%u%%", &percent) != 1)
    {
        return 0;
    }

    if (percent >= 100)
    {
        return 4;
    }

    return (uint8_t)((percent + 24U) / 25U);
}

static void ui_home_update_battery(const ui_home_view_model_t *model)
{
    uint8_t filled = 0;
    size_t index;

    if (model != NULL && model->has_battery)
    {
        filled = ui_home_battery_segments_filled(model->battery_badge_text);
        lv_label_set_text(s_home.battery_percent_label, model->battery_badge_text);
    }
    else
    {
        lv_label_set_text(s_home.battery_percent_label, "--%");
    }

    for (index = 0; index < 4; ++index)
    {
        lv_obj_set_style_bg_color(s_home.battery_segment[index],
                                  index < filled ? lv_color_white() : lv_color_black(),
                                  0);
        lv_obj_set_style_bg_opa(s_home.battery_segment[index],
                                index < filled ? LV_OPA_COVER : LV_OPA_TRANSP,
                                0);
    }
}

static void ui_home_update_wifi(const ui_home_view_model_t *model)
{
    uint8_t bars_to_show = 0;
    size_t index;

    if (model != NULL && model->has_wifi && model->wifi_connected)
    {
        bars_to_show = model->wifi_has_signal ? model->wifi_signal_bars : 1;
        if (bars_to_show > 3)
        {
            bars_to_show = 3;
        }
    }

    for (index = 0; index < 3; ++index)
    {
        lv_obj_set_style_bg_opa(s_home.wifi_bar[index], index < bars_to_show ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(s_home.wifi_bar[index], index < bars_to_show ? 0 : 1, 0);
    }
}

static void ui_home_update_marker(size_t index, uint16_t used_x100, uint16_t time_x100)
{
    lv_coord_t marker_center_x;
    lv_coord_t row_base_y;
    lv_coord_t marker_head_y;
    lv_coord_t fill_right_x;
    lv_color_t stem_color;

    row_base_y = QUOTA_ROW_BASE_Y[index];
    marker_head_y = (lv_coord_t)(row_base_y + UI_HOME_ROW_MARKER_HEAD_Y_OFFSET);
    marker_center_x = (lv_coord_t)(UI_HOME_BAR_X + (int32_t)((time_x100 * UI_HOME_BAR_WIDTH) / 10000U));
    if (marker_center_x < UI_HOME_BAR_X)
    {
        marker_center_x = UI_HOME_BAR_X;
    }
    if (marker_center_x > (UI_HOME_BAR_X + UI_HOME_BAR_WIDTH - 1))
    {
        marker_center_x = UI_HOME_BAR_X + UI_HOME_BAR_WIDTH - 1;
    }

    fill_right_x = (lv_coord_t)(UI_HOME_BAR_X + (int32_t)((used_x100 * UI_HOME_BAR_WIDTH) / 10000U));
    stem_color = marker_center_x <= fill_right_x ? lv_color_white() : lv_color_black();

    lv_obj_set_pos(s_home.quota_row[index].marker_head[0], marker_center_x - 4, marker_head_y);
    lv_obj_set_pos(s_home.quota_row[index].marker_head[1], marker_center_x - 3, marker_head_y + 1);
    lv_obj_set_pos(s_home.quota_row[index].marker_head[2], marker_center_x - 2, marker_head_y + 2);
    lv_obj_set_pos(s_home.quota_row[index].marker_stem,
                   marker_center_x,
                   (lv_coord_t)(row_base_y + UI_HOME_ROW_MARKER_STEM_Y_OFFSET));
    lv_obj_set_style_bg_color(s_home.quota_row[index].marker_stem, stem_color, 0);
    ui_home_set_hidden(s_home.quota_row[index].marker_head[0], false);
    ui_home_set_hidden(s_home.quota_row[index].marker_head[1], false);
    ui_home_set_hidden(s_home.quota_row[index].marker_head[2], false);
    ui_home_set_hidden(s_home.quota_row[index].marker_stem, false);
}

static void ui_home_hide_marker(size_t index)
{
    ui_home_set_hidden(s_home.quota_row[index].marker_head[0], true);
    ui_home_set_hidden(s_home.quota_row[index].marker_head[1], true);
    ui_home_set_hidden(s_home.quota_row[index].marker_head[2], true);
    ui_home_set_hidden(s_home.quota_row[index].marker_stem, true);
}

static void ui_home_update_bottom_row(const ui_home_view_model_t *model)
{
    char synced_text[UI_HOME_TEXT_LEN];
    char next_text[UI_HOME_TEXT_LEN];
    const char *status_text = "Idle";

    if (model != NULL && model->has_provider_status && model->provider_status_text[0] != '\0')
    {
        status_text = model->provider_status_text;
    }

    lv_label_set_text(s_home.bottom_status_label, status_text);
    lv_obj_set_style_bg_color(s_home.bottom_status_indicator,
                              (model != NULL && model->has_provider_status && model->provider_status_ok)
                                  ? lv_color_black()
                                  : lv_color_white(),
                              0);

    snprintf(synced_text,
             sizeof(synced_text),
             "Synced %.5s",
             (model != NULL && model->has_provider_last_sync && model->provider_last_sync_text[0] != '\0')
                 ? model->provider_last_sync_text
                 : "--:--");
    snprintf(next_text,
             sizeof(next_text),
             "Next %.5s",
             (model != NULL && model->has_provider_next_attempt && model->provider_next_attempt_text[0] != '\0')
                 ? model->provider_next_attempt_text
                 : "--:--");

    lv_label_set_text(s_home.bottom_synced_label, synced_text);
    lv_label_set_text(s_home.bottom_next_label, next_text);
}

static void ui_home_hide_quota_row(size_t index)
{
    size_t segment_index;

    ui_home_set_hidden(s_home.quota_row[index].label, true);
    ui_home_set_hidden(s_home.quota_row[index].amount, true);
    ui_home_set_hidden(s_home.quota_row[index].percent, true);
    ui_home_set_hidden(s_home.quota_row[index].bar, true);
    ui_home_set_hidden(s_home.quota_row[index].detail_left, true);
    ui_home_set_hidden(s_home.quota_row[index].detail_right, true);
    ui_home_set_hidden(s_home.divider[index], true);

    for (segment_index = 0; segment_index < 3; ++segment_index)
    {
        ui_home_set_hidden(s_home.quota_row[index].tick[segment_index], true);
    }

    ui_home_hide_marker(index);

    for (segment_index = 0; segment_index < UI_HOME_MAX_QUOTA_SEGMENTS; ++segment_index)
    {
        ui_home_set_hidden(s_home.quota_row[index].segment[segment_index], true);
    }
}

static void ui_home_update_quota_row(size_t index, const ui_home_view_model_t *model)
{
    size_t tick_index;
    size_t segment_index;
    lv_coord_t row_base_y;

    if (model == NULL || index >= model->quota_bar_count)
    {
        ui_home_hide_quota_row(index);
        return;
    }

    row_base_y = QUOTA_ROW_BASE_Y[index];

    ui_home_set_hidden(s_home.quota_row[index].label, false);
    ui_home_set_hidden(s_home.quota_row[index].percent, false);
    ui_home_set_hidden(s_home.quota_row[index].detail_left, true);
    ui_home_set_hidden(s_home.quota_row[index].detail_right, true);
    ui_home_set_hidden(s_home.divider[index], true);

    lv_label_set_text(s_home.quota_row[index].label, model->quota_bar_label[index]);
    lv_label_set_text(s_home.quota_row[index].percent,
                      model->quota_bar_percent_text[index][0] != '\0'
                          ? model->quota_bar_percent_text[index]
                          : model->quota_bar_value_text[index]);
    if (model->quota_bar_amount_text[index][0] != '\0')
    {
        ui_home_set_hidden(s_home.quota_row[index].amount, false);
        lv_label_set_text(s_home.quota_row[index].amount, model->quota_bar_amount_text[index]);
    }
    else if (model->quota_bar_detail_right_text[index][0] != '\0')
    {
        ui_home_set_hidden(s_home.quota_row[index].amount, false);
        lv_label_set_text(s_home.quota_row[index].amount, model->quota_bar_detail_right_text[index]);
    }
    else
    {
        ui_home_set_hidden(s_home.quota_row[index].amount, true);
    }

    if (model->quota_bar_segmented[index])
    {
        const lv_coord_t gap = 3;
        const lv_coord_t total = model->quota_bar_segment_total[index] > 0 ? model->quota_bar_segment_total[index] : 1;
        const lv_coord_t segment_width = (lv_coord_t)((UI_HOME_BAR_WIDTH - ((total - 1) * gap)) / total);
        const lv_coord_t segment_y = (lv_coord_t)(row_base_y + UI_HOME_ROW_BAR_Y_OFFSET);

        ui_home_set_hidden(s_home.quota_row[index].bar, true);
        for (tick_index = 0; tick_index < 3; ++tick_index)
        {
            ui_home_set_hidden(s_home.quota_row[index].tick[tick_index], true);
        }
        ui_home_hide_marker(index);

        for (segment_index = 0; segment_index < UI_HOME_MAX_QUOTA_SEGMENTS; ++segment_index)
        {
            if (segment_index < model->quota_bar_segment_total[index])
            {
                lv_coord_t segment_x = (lv_coord_t)(UI_HOME_BAR_X + (segment_index * (segment_width + gap)));
                lv_obj_set_pos(s_home.quota_row[index].segment[segment_index], segment_x, segment_y);
                lv_obj_set_size(s_home.quota_row[index].segment[segment_index], segment_width, UI_HOME_SEGMENT_BAR_HEIGHT);
                lv_obj_set_style_bg_color(s_home.quota_row[index].segment[segment_index],
                                          segment_index < model->quota_bar_segment_used[index] ? lv_color_black() : lv_color_white(),
                                          0);
                lv_obj_set_style_bg_opa(s_home.quota_row[index].segment[segment_index],
                                        segment_index < model->quota_bar_segment_used[index] ? LV_OPA_COVER : LV_OPA_TRANSP,
                                        0);
                ui_home_set_hidden(s_home.quota_row[index].segment[segment_index], false);
            }
            else
            {
                ui_home_set_hidden(s_home.quota_row[index].segment[segment_index], true);
            }
        }
        return;
    }

    ui_home_set_hidden(s_home.quota_row[index].bar, false);
    lv_bar_set_value(s_home.quota_row[index].bar, model->quota_bar_used_x100[index], LV_ANIM_OFF);

    for (tick_index = 0; tick_index < 3; ++tick_index)
    {
        lv_color_t tick_color = QUOTA_TICK_PCT[tick_index] <= model->quota_bar_used_x100[index]
                                    ? lv_color_white()
                                    : lv_color_black();
        ui_home_set_hidden(s_home.quota_row[index].tick[tick_index], false);
        lv_obj_set_style_bg_color(s_home.quota_row[index].tick[tick_index], tick_color, 0);
    }

    for (segment_index = 0; segment_index < UI_HOME_MAX_QUOTA_SEGMENTS; ++segment_index)
    {
        ui_home_set_hidden(s_home.quota_row[index].segment[segment_index], true);
    }

    if (model->quota_bar_has_time_marker[index])
    {
        ui_home_update_marker(index, model->quota_bar_used_x100[index], model->quota_bar_time_x100[index]);
    }
    else
    {
        ui_home_hide_marker(index);
    }
}

bool ui_home_screen_init(void)
{
    size_t index;

    if (s_home.initialized)
    {
        return true;
    }

    memset(&s_home, 0, sizeof(s_home));
    s_home.screen = lv_screen_active();
    if (s_home.screen == NULL)
    {
        return false;
    }

    lv_obj_clean(s_home.screen);
    lv_obj_set_style_bg_color(s_home.screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(s_home.screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_home.screen, 0, 0);

    s_home.status_bar = ui_home_create_rect(s_home.screen,
                                            0,
                                            0,
                                            UI_HOME_SCREEN_WIDTH,
                                            UI_HOME_STATUS_BAR_HEIGHT,
                                            lv_color_black(),
                                            LV_OPA_COVER,
                                            lv_color_black(),
                                            0);
    s_home.content = ui_home_create_rect(s_home.screen,
                                         0,
                                         UI_HOME_STATUS_BAR_HEIGHT,
                                         UI_HOME_SCREEN_WIDTH,
                                         UI_HOME_CONTENT_HEIGHT,
                                         lv_color_white(),
                                         LV_OPA_COVER,
                                         lv_color_white(),
                                         0);

    s_home.time_label = ui_home_create_label(s_home.status_bar, TIME_FONT, lv_color_white(), 7, 4, 72, LV_TEXT_ALIGN_LEFT);
    s_home.date_label = ui_home_create_label(s_home.status_bar, STATUS_FONT, lv_color_white(), 82, 4, 46, LV_TEXT_ALIGN_LEFT);
    s_home.weekday_label = ui_home_create_label(s_home.status_bar, STATUS_FONT, lv_color_white(), 82, 19, 46, LV_TEXT_ALIGN_LEFT);

    s_home.provider_badge = ui_home_create_rect(s_home.status_bar,
                                                145,
                                                6,
                                                114,
                                                22,
                                                lv_color_black(),
                                                LV_OPA_COVER,
                                                lv_color_white(),
                                                1);
    s_home.provider_badge_label = ui_home_create_label(s_home.provider_badge, STATUS_FONT, lv_color_white(), 0, 3, 114, LV_TEXT_ALIGN_CENTER);

    s_home.environment_label = ui_home_create_label(s_home.status_bar, STATUS_FONT, lv_color_white(), 264, 8, 52, LV_TEXT_ALIGN_LEFT);
    ui_home_set_hidden(s_home.environment_label, true);

    s_home.battery_body = ui_home_create_rect(s_home.status_bar,
                                              319,
                                              8,
                                              25,
                                              14,
                                              lv_color_black(),
                                              LV_OPA_COVER,
                                              lv_color_white(),
                                              1);
    for (index = 0; index < 4; ++index)
    {
        s_home.battery_segment[index] = ui_home_create_rect(s_home.battery_body,
                                                            (lv_coord_t)(1 + (index * 6)),
                                                            2,
                                                            4,
                                                            10,
                                                            lv_color_white(),
                                                            LV_OPA_TRANSP,
                                                            lv_color_white(),
                                                            0);
    }
    s_home.battery_tip = ui_home_create_rect(s_home.status_bar,
                                             345,
                                             12,
                                             2,
                                             6,
                                             lv_color_white(),
                                             LV_OPA_COVER,
                                             lv_color_white(),
                                             0);
    s_home.battery_percent_label = ui_home_create_label(s_home.status_bar, STATUS_FONT, lv_color_white(), 350, 8, 27, LV_TEXT_ALIGN_LEFT);

    s_home.wifi_bar[0] = ui_home_create_rect(s_home.status_bar, 384, 16, 3, 6, lv_color_white(), LV_OPA_TRANSP, lv_color_white(), 1);
    s_home.wifi_bar[1] = ui_home_create_rect(s_home.status_bar, 388, 12, 3, 10, lv_color_white(), LV_OPA_TRANSP, lv_color_white(), 1);
    s_home.wifi_bar[2] = ui_home_create_rect(s_home.status_bar, 392, 8, 3, 14, lv_color_white(), LV_OPA_TRANSP, lv_color_white(), 1);

    for (index = 0; index < UI_HOME_MAX_QUOTA_BARS; ++index)
    {
        size_t tick_index;
        size_t segment_index;
        lv_coord_t row_base_y = QUOTA_ROW_BASE_Y[index];
        lv_coord_t bar_y = (lv_coord_t)(row_base_y + UI_HOME_ROW_BAR_Y_OFFSET);

        s_home.quota_row[index].label = ui_home_create_label(s_home.content,
                                                             LABEL_FONT,
                                                             lv_color_black(),
                                                             UI_HOME_BAR_X,
                                                             row_base_y,
                                                             QUOTA_LABEL_WIDTH,
                                                             LV_TEXT_ALIGN_LEFT);
        s_home.quota_row[index].amount = ui_home_create_label(s_home.content,
                                                              LABEL_FONT,
                                                              lv_color_black(),
                                                              QUOTA_AMOUNT_X,
                                                              row_base_y + 1,
                                                              QUOTA_AMOUNT_WIDTH,
                                                              LV_TEXT_ALIGN_RIGHT);
        s_home.quota_row[index].percent = ui_home_create_label(s_home.content,
                                                               LABEL_FONT,
                                                               lv_color_black(),
                                                               QUOTA_PERCENT_X,
                                                               row_base_y,
                                                               QUOTA_PERCENT_WIDTH,
                                                               LV_TEXT_ALIGN_RIGHT);
        s_home.quota_row[index].bar = ui_home_create_quota_bar(s_home.content, UI_HOME_BAR_X, bar_y);
        s_home.quota_row[index].detail_left = ui_home_create_label(s_home.content,
                                                                   TEXT_FONT,
                                                                   lv_color_black(),
                                                                   UI_HOME_BAR_X,
                                                                   row_base_y + UI_HOME_ROW_DETAIL_Y_OFFSET,
                                                                   150,
                                                                   LV_TEXT_ALIGN_LEFT);
        s_home.quota_row[index].detail_right = ui_home_create_label(s_home.content,
                                                                    TEXT_FONT,
                                                                    lv_color_black(),
                                                                    230,
                                                                    row_base_y + UI_HOME_ROW_DETAIL_Y_OFFSET,
                                                                    140,
                                                                    LV_TEXT_ALIGN_RIGHT);

        for (tick_index = 0; tick_index < 3; ++tick_index)
        {
            lv_coord_t tick_x = (lv_coord_t)(UI_HOME_BAR_X + ((QUOTA_TICK_PCT[tick_index] * UI_HOME_BAR_WIDTH) / 10000U));
            s_home.quota_row[index].tick[tick_index] = ui_home_create_rect(s_home.content,
                                                                           tick_x,
                                                                           bar_y,
                                                                           1,
                                                                           UI_HOME_BAR_HEIGHT,
                                                                           lv_color_black(),
                                                                           LV_OPA_COVER,
                                                                           lv_color_black(),
                                                                           0);
        }

        s_home.quota_row[index].marker_head[0] = ui_home_create_rect(s_home.content, 0, 0, 9, 1, lv_color_black(), LV_OPA_COVER, lv_color_black(), 0);
        s_home.quota_row[index].marker_head[1] = ui_home_create_rect(s_home.content, 0, 0, 7, 1, lv_color_black(), LV_OPA_COVER, lv_color_black(), 0);
        s_home.quota_row[index].marker_head[2] = ui_home_create_rect(s_home.content, 0, 0, 5, 1, lv_color_black(), LV_OPA_COVER, lv_color_black(), 0);
        s_home.quota_row[index].marker_stem = ui_home_create_rect(s_home.content, 0, 0, 1, UI_HOME_BAR_HEIGHT, lv_color_black(), LV_OPA_COVER, lv_color_black(), 0);
        ui_home_hide_marker(index);

        for (segment_index = 0; segment_index < UI_HOME_MAX_QUOTA_SEGMENTS; ++segment_index)
        {
            s_home.quota_row[index].segment[segment_index] = ui_home_create_rect(s_home.content,
                                                                                 UI_HOME_BAR_X,
                                                                                 bar_y,
                                                                                 20,
                                                                                 UI_HOME_SEGMENT_BAR_HEIGHT,
                                                                                 lv_color_white(),
                                                                                 LV_OPA_TRANSP,
                                                                                 lv_color_black(),
                                                                                 1);
            ui_home_set_hidden(s_home.quota_row[index].segment[segment_index], true);
        }

        s_home.divider[index] = ui_home_create_rect(s_home.content,
                                                    UI_HOME_BAR_X,
                                                    row_base_y + UI_HOME_ROW_DIVIDER_Y_OFFSET,
                                                    UI_HOME_BAR_WIDTH,
                                                    1,
                                                    lv_color_black(),
                                                    LV_OPA_COVER,
                                                    lv_color_black(),
                                                    0);
    }

    s_home.empty_label = ui_home_create_label(s_home.content, TEXT_FONT, lv_color_black(), 40, 116, 320, LV_TEXT_ALIGN_CENTER);
    ui_home_set_hidden(s_home.empty_label, true);

    s_home.bottom_divider = ui_home_create_rect(s_home.content,
                                                UI_HOME_BAR_X,
                                                UI_HOME_BOTTOM_ROW_Y - 12,
                                                UI_HOME_BAR_WIDTH,
                                                1,
                                                lv_color_black(),
                                                LV_OPA_COVER,
                                                lv_color_black(),
                                                0);
    s_home.bottom_status_indicator = ui_home_create_rect(s_home.content,
                                                         UI_HOME_BAR_X,
                                                         UI_HOME_BOTTOM_ROW_Y - 2,
                                                         6,
                                                         6,
                                                         lv_color_black(),
                                                         LV_OPA_COVER,
                                                         lv_color_black(),
                                                         1);
    s_home.bottom_status_label = ui_home_create_label(s_home.content,
                                                      TEXT_FONT,
                                                      lv_color_black(),
                                                      UI_HOME_BAR_X + 10,
                                                      UI_HOME_BOTTOM_ROW_Y - 6,
                                                      82,
                                                      LV_TEXT_ALIGN_LEFT);
    s_home.bottom_synced_label = ui_home_create_label(s_home.content,
                                                      TEXT_FONT,
                                                      lv_color_black(),
                                                      204,
                                                      UI_HOME_BOTTOM_ROW_Y - 6,
                                                      92,
                                                      LV_TEXT_ALIGN_RIGHT);
    s_home.bottom_next_label = ui_home_create_label(s_home.content,
                                                    TEXT_FONT,
                                                    lv_color_black(),
                                                    300,
                                                    UI_HOME_BOTTOM_ROW_Y - 6,
                                                    80,
                                                    LV_TEXT_ALIGN_RIGHT);

    s_home.initialized = true;
    return true;
}

void ui_home_screen_reset(void)
{
    memset(&s_home, 0, sizeof(s_home));
}

void ui_home_screen_update(const ui_home_view_model_t *model)
{
    size_t index;

    if (model == NULL || !s_home.initialized)
    {
        return;
    }

    lv_label_set_text(s_home.time_label, model->has_time ? model->time_text : "--:--");
    lv_label_set_text(s_home.date_label, model->has_time ? model->date_text : "--/--");
    lv_label_set_text(s_home.weekday_label,
                      (model->has_time && model->weekday_text[0] != '\0') ? model->weekday_text : "---");

    if (model->has_provider && model->provider_name_text[0] != '\0')
    {
        lv_label_set_text(s_home.provider_badge_label, model->provider_name_text);
    }
    else
    {
        lv_label_set_text(s_home.provider_badge_label, model->configured ? "IDLE" : "SETUP");
    }

    lv_label_set_text(s_home.environment_label,
                      model->has_environment ? model->environment_text : "--C --%");
    ui_home_update_battery(model);
    ui_home_update_wifi(model);

    for (index = 0; index < UI_HOME_MAX_QUOTA_BARS; ++index)
    {
        ui_home_update_quota_row(index, model);
    }

    if (model->quota_bar_count == 0)
    {
        lv_label_set_text(s_home.empty_label,
                          model->configured ? "Waiting for provider data" : "Complete setup to start");
        ui_home_set_hidden(s_home.empty_label, false);
    }
    else
    {
        ui_home_set_hidden(s_home.empty_label, true);
    }

    ui_home_update_bottom_row(model);
}