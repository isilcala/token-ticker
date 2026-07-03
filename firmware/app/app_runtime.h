#ifndef TOKEN_TICKER_APP_RUNTIME_H
#define TOKEN_TICKER_APP_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

#include "app_bootstrap.h"

void app_runtime_step(app_bootstrap_context_t *context, int64_t now_epoch_seconds);
bool app_runtime_start(app_bootstrap_context_t *context);

#endif