#include "ui_sleep_screen.h"

#include "lvgl.h"

static bool ui_sleep_screen_show_text(const char *title_text, const char *detail_text)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_t *title;
    lv_obj_t *detail;

    if (screen == NULL)
    {
        return false;
    }

    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(screen, 0, 0);

    title = lv_label_create(screen);
    lv_label_set_text(title, title_text);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -18);

    detail = lv_label_create(screen);
    lv_label_set_text(detail, detail_text);
    lv_obj_set_style_text_font(detail, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(detail, lv_color_black(), 0);
    lv_obj_align(detail, LV_ALIGN_CENTER, 0, 20);

    return true;
}

bool ui_sleep_screen_show(void)
{
    return ui_sleep_screen_show_text("SLEEPING", "Press KEY to wake");
}

bool ui_sleep_screen_show_waking(void)
{
    return ui_sleep_screen_show_text("WAKING", "Refreshing data...");
}