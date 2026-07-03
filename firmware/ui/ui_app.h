#ifndef TOKEN_TICKER_UI_APP_H
#define TOKEN_TICKER_UI_APP_H

#include <stdint.h>

#include "board.h"
#include "ui_boot_model.h"

void ui_app_show_boot_status(const board_config_t *board,
                             const char *phase,
                             const char *detail,
                             uint8_t progress_percent);
void ui_app_boot(const ui_boot_model_t *model);
void ui_app_update(const ui_boot_model_t *model);

#endif