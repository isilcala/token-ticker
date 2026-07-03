#ifndef TOKEN_TICKER_MINIMAX_PROVIDER_H
#define TOKEN_TICKER_MINIMAX_PROVIDER_H

#include "app_config.h"
#include "provider_interface.h"
#include "minimax_quota_types.h"

const provider_interface_t *minimax_provider_get_interface(void);
bool minimax_provider_fetch_snapshot(const provider_config_t *config, provider_snapshot_t *snapshot);

#endif