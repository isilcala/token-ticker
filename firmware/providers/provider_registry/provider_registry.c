#include "provider_registry.h"

#include "minimax_provider.h"

static const provider_interface_t *s_active_provider;
static const provider_config_t *s_active_provider_config;

const provider_interface_t *provider_registry_resolve_type(provider_type_t provider_type)
{
    switch (provider_type)
    {
    case PROVIDER_TYPE_MINIMAX:
        return minimax_provider_get_interface();
    case PROVIDER_TYPE_UNKNOWN:
    default:
        return NULL;
    }
}

void provider_registry_init(const app_config_t *config)
{
    const provider_config_t *active_config;

    s_active_provider = NULL;
    s_active_provider_config = NULL;

    if (config == NULL)
    {
        return;
    }

    active_config = app_config_find_active_provider(config);
    if (active_config == NULL)
    {
        return;
    }

    s_active_provider_config = active_config;
    s_active_provider = provider_registry_resolve_type(active_config->provider_type);
}

bool provider_registry_has_active(void)
{
    return s_active_provider != NULL;
}

const provider_interface_t *provider_registry_get_active(void)
{
    return s_active_provider;
}

const provider_config_t *provider_registry_get_active_config(void)
{
    return s_active_provider_config;
}

bool provider_registry_fetch_active_snapshot(provider_snapshot_t *snapshot)
{
    if (snapshot == NULL || s_active_provider_config == NULL || s_active_provider == NULL)
    {
        return false;
    }

    return s_active_provider->fetch_snapshot(s_active_provider_config, snapshot);
}