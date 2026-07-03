#ifndef TOKEN_TICKER_APP_KEY_INPUT_H
#define TOKEN_TICKER_APP_KEY_INPUT_H

#include <stdbool.h>

#include "board.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool app_key_input_init(const board_config_t *board);
void app_key_input_register_task(TaskHandle_t task_handle);
bool app_key_input_consume_press(void);

#endif