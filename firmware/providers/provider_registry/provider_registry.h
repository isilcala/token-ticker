#ifndef TOKEN_TICKER_PROVIDER_REGISTRY_H
#define TOKEN_TICKER_PROVIDER_REGISTRY_H

#include <stdbool.h>

#include "app_config.h"
#include "provider_interface.h"

void provider_registry_init(const app_config_t *config);
bool provider_registry_has_active(void);
const provider_interface_t *provider_registry_get_active(void);
const provider_config_t *provider_registry_get_active_config(void);
bool provider_registry_fetch_active_snapshot(provider_snapshot_t *snapshot);
const provider_interface_t *provider_registry_resolve_type(provider_type_t provider_type);

#endif