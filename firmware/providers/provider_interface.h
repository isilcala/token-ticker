#ifndef TOKEN_TICKER_PROVIDER_INTERFACE_H
#define TOKEN_TICKER_PROVIDER_INTERFACE_H

#include <stdbool.h>

#include "app_config.h"
#include "provider_snapshot.h"

typedef struct
{
    const char *provider_id;
    const char *display_name;
    provider_type_t provider_type;
    bool (*fetch_snapshot)(const provider_config_t *config, provider_snapshot_t *snapshot);
} provider_interface_t;

#endif