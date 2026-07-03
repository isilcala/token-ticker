#include "minimax_quota_types.h"

#include <stdio.h>
#include <string.h>

static void minimax_copy_text(char *target, size_t target_len, const char *value)
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

static int32_t minimax_compute_remaining(int32_t total, int32_t used)
{
    const int32_t remaining = total - used;
    return remaining > 0 ? remaining : 0;
}

static int32_t minimax_compute_percent_x100(int32_t total, int32_t used, int32_t provided_percent_x100)
{
    if (provided_percent_x100 > 0)
    {
        return provided_percent_x100;
    }

    if (total <= 0)
    {
        return 0;
    }

    return (minimax_compute_remaining(total, used) * 10000) / total;
}

static int32_t minimax_compute_used_percent_x100(int32_t total, int32_t used, int32_t remaining_percent_x100)
{
    if (remaining_percent_x100 > 0)
    {
        const int32_t used_percent_x100 = 10000 - remaining_percent_x100;
        return used_percent_x100 > 0 ? used_percent_x100 : 0;
    }

    if (total <= 0)
    {
        return 0;
    }

    return (used * 10000) / total;
}

static void minimax_set_window_label_for_current(const minimax_model_remain_t *model,
                                                 char *target,
                                                 size_t target_len)
{
    const int64_t interval_hours = (model->end_time_ms - model->start_time_ms) / (1000LL * 60LL * 60LL);

    if (target == NULL || target_len == 0 || model == NULL)
    {
        return;
    }

    target[0] = '\0';

    if (model->current_interval_total_count > 0)
    {
        if (strcmp(model->model_name, "video") == 0)
        {
            snprintf(target, target_len, "video/day");
            return;
        }

        snprintf(target, target_len, "%s", model->model_name);
        return;
    }

    if (strcmp(model->model_name, "general") == 0 && interval_hours > 0)
    {
        snprintf(target, target_len, "%lldh quota", (long long)interval_hours);
        return;
    }

    snprintf(target, target_len, "%s", model->model_name);
}

static void minimax_set_window_label_for_weekly(const minimax_model_remain_t *model,
                                                char *target,
                                                size_t target_len)
{
    if (target == NULL || target_len == 0 || model == NULL)
    {
        return;
    }

    target[0] = '\0';

    if (strcmp(model->model_name, "video") == 0)
    {
        snprintf(target, target_len, "video/week");
        return;
    }

    if (strcmp(model->model_name, "general") == 0)
    {
        snprintf(target, target_len, "weekly quota");
        return;
    }

    snprintf(target, target_len, "%s week", model->model_name);
}

static bool minimax_append_window(provider_snapshot_t *snapshot,
                                  const char *id,
                                  const char *label,
                                  int32_t used,
                                  int32_t limit,
                                  int32_t remaining,
                                  int32_t remaining_percent_x100,
                                  int64_t start_epoch,
                                  int64_t reset_epoch,
                                  quota_window_status_t status)
{
    quota_window_t *window;

    if (snapshot == NULL || id == NULL || label == NULL || snapshot->window_count >= PROVIDER_SNAPSHOT_MAX_WINDOWS)
    {
        return false;
    }

    window = &snapshot->windows[snapshot->window_count];
    minimax_copy_text(window->id, sizeof(window->id), id);
    minimax_copy_text(window->label, sizeof(window->label), label);
    window->used = used;
    window->limit = limit;
    window->remaining = remaining;
    window->remaining_percent_x100 = remaining_percent_x100;
    window->start_epoch = start_epoch;
    window->reset_epoch = reset_epoch;
    window->status = status;
    ++snapshot->window_count;
    return true;
}

quota_window_status_t minimax_status_to_window_status(int32_t provider_status)
{
    switch (provider_status)
    {
    case 1:
        return QUOTA_WINDOW_STATUS_LIMITED;
    case 2:
        return QUOTA_WINDOW_STATUS_EXHAUSTED;
    case 3:
        return QUOTA_WINDOW_STATUS_UNLIMITED;
    default:
        return QUOTA_WINDOW_STATUS_UNKNOWN;
    }
}

bool minimax_map_quota_response(const provider_config_t *config,
                                const minimax_quota_response_t *response,
                                provider_snapshot_t *snapshot)
{
    size_t index;

    if (config == NULL || response == NULL || snapshot == NULL || response->model_count == 0)
    {
        return false;
    }

    provider_snapshot_init(snapshot);
    snapshot->provider_type = PROVIDER_TYPE_MINIMAX;
    snapshot->sync_state = PROVIDER_SYNC_STATE_OK;
    snapshot->stale = false;
    snapshot->capabilities.supports_multiple_windows = true;
    snapshot->capabilities.supports_manual_refresh = true;
    snapshot->capabilities.supports_region_label = true;

    minimax_copy_text(snapshot->provider_id, sizeof(snapshot->provider_id), config->id);
    minimax_copy_text(snapshot->display_name,
                      sizeof(snapshot->display_name),
                      (config->region[0] != '\0' && strcmp(config->region, "cn") == 0) ? "MiniMax CN" : "MiniMax");
    minimax_copy_text(snapshot->region, sizeof(snapshot->region), config->region);

    snapshot->primary_window_index = 0;

    for (index = 0; index < response->model_count && snapshot->window_count < PROVIDER_SNAPSHOT_MAX_WINDOWS; ++index)
    {
        const minimax_model_remain_t *model = &response->models[index];
        char label[PROVIDER_SNAPSHOT_LABEL_LEN];

        if (model->current_interval_total_count > 0 || model->current_interval_remaining_percent_x100 > 0)
        {
            const int32_t used_percent_x100 = minimax_compute_used_percent_x100(model->current_interval_total_count,
                                                                                model->current_interval_usage_count,
                                                                                model->current_interval_remaining_percent_x100);

            minimax_set_window_label_for_current(model, label, sizeof(label));
            if (model->current_interval_total_count > 0)
            {
                (void)minimax_append_window(snapshot,
                                            strcmp(model->model_name, "video") == 0 ? "video_day" : "current",
                                            label,
                                            model->current_interval_usage_count,
                                            model->current_interval_total_count,
                                            minimax_compute_remaining(model->current_interval_total_count,
                                                                      model->current_interval_usage_count),
                                            minimax_compute_percent_x100(model->current_interval_total_count,
                                                                         model->current_interval_usage_count,
                                                                         model->current_interval_remaining_percent_x100),
                                            model->start_time_ms / 1000,
                                            model->end_time_ms / 1000,
                                            minimax_status_to_window_status(model->current_interval_status));
            }
            else
            {
                const int32_t used_percent_whole = (used_percent_x100 + 50) / 100;
                const int32_t remaining_percent_whole = (model->current_interval_remaining_percent_x100 + 50) / 100;

                (void)minimax_append_window(snapshot,
                                            "general_5h",
                                            label,
                                            used_percent_whole,
                                            100,
                                            remaining_percent_whole,
                                            model->current_interval_remaining_percent_x100,
                                            model->start_time_ms / 1000,
                                            model->end_time_ms / 1000,
                                            minimax_status_to_window_status(model->current_interval_status));
            }
        }

        if (strcmp(model->model_name, "general") == 0)
        {
            char label[PROVIDER_SNAPSHOT_LABEL_LEN];
            const int32_t used_percent_x100 = minimax_compute_used_percent_x100(model->current_weekly_total_count,
                                                                                model->current_weekly_usage_count,
                                                                                model->current_weekly_remaining_percent_x100);
            const int32_t used_percent_whole = (used_percent_x100 + 50) / 100;
            const int32_t remaining_percent_whole = (model->current_weekly_remaining_percent_x100 + 50) / 100;

            minimax_set_window_label_for_weekly(model, label, sizeof(label));
            (void)minimax_append_window(snapshot,
                                        "general_week",
                                        label,
                                        used_percent_whole,
                                        100,
                                        remaining_percent_whole,
                                        model->current_weekly_remaining_percent_x100,
                                        model->weekly_start_time_ms / 1000,
                                        model->weekly_end_time_ms / 1000,
                                        minimax_status_to_window_status(model->current_weekly_status));
        }
        else if (strcmp(model->model_name, "video") == 0 && model->current_interval_total_count > 0)
        {
            /* video weekly intentionally omitted from the home screen to keep
               the quota area focused on 5h/general-week/video-day. */
        }

        if (index == 0)
        {
            snapshot->last_success_epoch = model->end_time_ms / 1000;
        }
    }

    snapshot->metric_count = 2;
    minimax_copy_text(snapshot->metrics[0].label, sizeof(snapshot->metrics[0].label), "models");
    snprintf(snapshot->metrics[0].value_text,
             sizeof(snapshot->metrics[0].value_text),
             "%u",
             (unsigned)response->model_count);
    snapshot->metrics[0].priority = 1;

    minimax_copy_text(snapshot->metrics[1].label, sizeof(snapshot->metrics[1].label), "window_count");
    snprintf(snapshot->metrics[1].value_text,
             sizeof(snapshot->metrics[1].value_text),
             "%u",
             (unsigned)snapshot->window_count);
    snapshot->metrics[1].priority = 2;

    return true;
}