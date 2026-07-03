#include "minimax_provider.h"

#include <stdio.h>
#include <string.h>

#include "minimax_http.h"

static void minimax_set_text(char *target, size_t target_len, const char *value)
{
    if (target == NULL || target_len == 0)
    {
        return;
    }

    target[0] = '\0';
    if (value != NULL)
    {
        strncpy(target, value, target_len - 1);
        target[target_len - 1] = '\0';
    }
}

bool minimax_provider_fetch_snapshot(const provider_config_t *config, provider_snapshot_t *snapshot)
{
    http_request_t request;
    http_response_meta_t meta;
    minimax_quota_response_t response;
    char response_json[2048];

    if (config == NULL || snapshot == NULL)
    {
        return false;
    }

    provider_snapshot_init(snapshot);
    snapshot->provider_type = PROVIDER_TYPE_MINIMAX;
    snapshot->stale = true;
    minimax_set_text(snapshot->provider_id, sizeof(snapshot->provider_id), config->id);
    minimax_set_text(snapshot->display_name,
                     sizeof(snapshot->display_name),
                     (config->region[0] != '\0' && strcmp(config->region, "cn") == 0) ? "MiniMax CN" : "MiniMax");
    minimax_set_text(snapshot->region, sizeof(snapshot->region), config->region);
    snapshot->capabilities.supports_multiple_windows = true;
    snapshot->capabilities.supports_manual_refresh = true;
    snapshot->capabilities.supports_region_label = true;

    if (!minimax_http_build_quota_request(config, &request))
    {
        snapshot->sync_state = PROVIDER_SYNC_STATE_AUTH_ERROR;
        snapshot->metric_count = 1;
        minimax_set_text(snapshot->metrics[0].label, sizeof(snapshot->metrics[0].label), "transport");
        minimax_set_text(snapshot->metrics[0].value_text, sizeof(snapshot->metrics[0].value_text), "credentials unavailable");
        snapshot->metrics[0].priority = 1;
        return true;
    }

    if (!http_client_get_json(&request, response_json, sizeof(response_json), &meta))
    {
        snapshot->sync_state = (meta.http_status_code == 401 || meta.http_status_code == 403)
                                   ? PROVIDER_SYNC_STATE_AUTH_ERROR
                                   : PROVIDER_SYNC_STATE_NETWORK_ERROR;
        snapshot->metric_count = 2;
        minimax_set_text(snapshot->metrics[0].label, sizeof(snapshot->metrics[0].label), "transport");
        snprintf(snapshot->metrics[0].value_text,
                 sizeof(snapshot->metrics[0].value_text),
                 "status=%d http=%u",
                 (int)meta.status,
                 meta.http_status_code);
        snapshot->metrics[0].priority = 1;
        minimax_set_text(snapshot->metrics[1].label, sizeof(snapshot->metrics[1].label), "endpoint");
        minimax_set_text(snapshot->metrics[1].value_text,
                         sizeof(snapshot->metrics[1].value_text),
                         request.url);
        snapshot->metrics[1].priority = 2;
        return true;
    }

    if (!minimax_http_parse_quota_response_json(response_json, &response))
    {
        snapshot->sync_state = PROVIDER_SYNC_STATE_PARSE_ERROR;
        snapshot->metric_count = 2;
        minimax_set_text(snapshot->metrics[0].label, sizeof(snapshot->metrics[0].label), "transport");
        minimax_set_text(snapshot->metrics[0].value_text, sizeof(snapshot->metrics[0].value_text), "json parse failed");
        snapshot->metrics[0].priority = 1;
        minimax_set_text(snapshot->metrics[1].label, sizeof(snapshot->metrics[1].label), "endpoint");
        minimax_set_text(snapshot->metrics[1].value_text,
                         sizeof(snapshot->metrics[1].value_text),
                         request.url);
        snapshot->metrics[1].priority = 2;
        return true;
    }

    if (!minimax_map_quota_response(config, &response, snapshot))
    {
        snapshot->sync_state = PROVIDER_SYNC_STATE_PARSE_ERROR;
        return true;
    }

    if (snapshot->metric_count + 2 <= PROVIDER_SNAPSHOT_MAX_METRICS)
    {
        size_t metric_index = snapshot->metric_count;
        minimax_set_text(snapshot->metrics[metric_index].label, sizeof(snapshot->metrics[metric_index].label), "endpoint");
        minimax_set_text(snapshot->metrics[metric_index].value_text,
                         sizeof(snapshot->metrics[metric_index].value_text),
                         request.url);
        snapshot->metrics[metric_index].priority = (uint8_t)(metric_index + 1);
        ++snapshot->metric_count;

        metric_index = snapshot->metric_count;
        minimax_set_text(snapshot->metrics[metric_index].label, sizeof(snapshot->metrics[metric_index].label), "http_status");
        snprintf(snapshot->metrics[metric_index].value_text,
                 sizeof(snapshot->metrics[metric_index].value_text),
                 "%u",
                 meta.http_status_code);
        snapshot->metrics[metric_index].priority = (uint8_t)(metric_index + 1);
        ++snapshot->metric_count;
    }

    return true;
}

static bool minimax_fetch_snapshot(const provider_config_t *config, provider_snapshot_t *snapshot)
{
    return minimax_provider_fetch_snapshot(config, snapshot);
}

static const provider_interface_t k_minimax_provider = {
    .provider_id = "minimax-cn",
    .display_name = "MiniMax CN",
    .provider_type = PROVIDER_TYPE_MINIMAX,
    .fetch_snapshot = minimax_fetch_snapshot,
};

const provider_interface_t *minimax_provider_get_interface(void)
{
    return &k_minimax_provider;
}