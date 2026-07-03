#include "minimax_http.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cJSON.h"

static const char *minimax_base_url_for_region(const char *region)
{
    if (region != NULL && strcmp(region, "cn") == 0)
    {
        return "https://api.minimaxi.com";
    }

    return "https://api.minimax.io";
}

bool minimax_http_build_quota_request(const provider_config_t *config, http_request_t *request)
{
    char url[HTTP_URL_MAX_LEN];
    int written;

    if (config == NULL || request == NULL || config->api_key[0] == '\0')
    {
        return false;
    }

    http_request_init(request);

    written = snprintf(url,
                       sizeof(url),
                       "%s/v1/token_plan/remains",
                       minimax_base_url_for_region(config->region));
    if (written <= 0 || (size_t)written >= sizeof(url))
    {
        return false;
    }

    return http_request_set_url(request, url) && http_request_set_bearer_token(request, config->api_key);
}

static bool minimax_get_required_number(const cJSON *object, const char *field, int64_t *value)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, field);
    if (!cJSON_IsNumber(item))
    {
        return false;
    }

    *value = (int64_t)item->valuedouble;
    return true;
}

static int32_t minimax_get_optional_percent_x100(const cJSON *object, const char *field)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, field);
    if (!cJSON_IsNumber(item))
    {
        return 0;
    }

    return (int32_t)llround(item->valuedouble * 100.0);
}

static int32_t minimax_get_optional_int32(const cJSON *object, const char *field, int32_t fallback)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(object, field);
    if (!cJSON_IsNumber(item))
    {
        return fallback;
    }

    return (int32_t)item->valuedouble;
}

static bool minimax_parse_model_item(const cJSON *item, minimax_model_remain_t *model)
{
    const cJSON *model_name = cJSON_GetObjectItemCaseSensitive(item, "model_name");
    int64_t value64;

    if (!cJSON_IsString(model_name) || model_name->valuestring == NULL)
    {
        return false;
    }

    memset(model, 0, sizeof(*model));
    strncpy(model->model_name, model_name->valuestring, sizeof(model->model_name) - 1);
    model->weekly_boost_permille = 1000;

    if (!minimax_get_required_number(item, "start_time", &model->start_time_ms) ||
        !minimax_get_required_number(item, "end_time", &model->end_time_ms) ||
        !minimax_get_required_number(item, "remains_time", &model->remains_time_ms) ||
        !minimax_get_required_number(item, "current_interval_total_count", &value64))
    {
        return false;
    }
    model->current_interval_total_count = (int32_t)value64;

    if (!minimax_get_required_number(item, "current_interval_usage_count", &value64))
    {
        return false;
    }
    model->current_interval_usage_count = (int32_t)value64;

    if (!minimax_get_required_number(item, "current_weekly_total_count", &value64))
    {
        return false;
    }
    model->current_weekly_total_count = (int32_t)value64;

    if (!minimax_get_required_number(item, "current_weekly_usage_count", &value64))
    {
        return false;
    }
    model->current_weekly_usage_count = (int32_t)value64;

    if (!minimax_get_required_number(item, "weekly_start_time", &model->weekly_start_time_ms) ||
        !minimax_get_required_number(item, "weekly_end_time", &model->weekly_end_time_ms) ||
        !minimax_get_required_number(item, "weekly_remains_time", &model->weekly_remains_time_ms))
    {
        return false;
    }

    model->current_interval_remaining_percent_x100 = minimax_get_optional_percent_x100(item, "current_interval_remaining_percent");
    model->current_weekly_remaining_percent_x100 = minimax_get_optional_percent_x100(item, "current_weekly_remaining_percent");
    model->current_interval_status = minimax_get_optional_int32(item, "current_interval_status", 0);
    model->current_weekly_status = minimax_get_optional_int32(item, "current_weekly_status", 0);
    model->weekly_boost_permille = minimax_get_optional_int32(item, "weekly_boost_permille", 1000);

    return true;
}

bool minimax_http_parse_quota_response_json(const char *json_text, minimax_quota_response_t *response)
{
    cJSON *root;
    const cJSON *array;
    const cJSON *item;
    size_t index = 0;

    if (json_text == NULL || response == NULL)
    {
        return false;
    }

    memset(response, 0, sizeof(*response));
    root = cJSON_Parse(json_text);
    if (root == NULL)
    {
        return false;
    }

    array = cJSON_GetObjectItemCaseSensitive(root, "model_remains");
    if (!cJSON_IsArray(array))
    {
        cJSON_Delete(root);
        return false;
    }

    cJSON_ArrayForEach(item, array)
    {
        if (index >= MINIMAX_MAX_MODELS)
        {
            break;
        }

        if (minimax_parse_model_item(item, &response->models[index]))
        {
            ++index;
        }
        else
        {
            cJSON_Delete(root);
            return false;
        }
    }

    response->model_count = index;
    cJSON_Delete(root);
    return response->model_count > 0;
}