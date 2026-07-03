#ifndef TOKEN_TICKER_UI_HOME_SCREEN_H
#define TOKEN_TICKER_UI_HOME_SCREEN_H

#include <stdbool.h>

#include "ui_home_view_model.h"

bool ui_home_screen_init(void);
void ui_home_screen_reset(void);
void ui_home_screen_update(const ui_home_view_model_t *model);

#endif