#include "config_store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "driver/sdmmc_host.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdmmc_cmd.h"

enum
{
    CONFIG_JSON_MAX_LEN = 4096,
};

static const char *CONFIG_NAMESPACE = "token_ticker";
static const char *CONFIG_KEY = "app_config_json";
static const char *CONFIG_SD_PATH = "/sdcard/token-ticker/config.json";
static const char *TAG = "config_store";
static sdmmc_card_t *s_sdcard;
static bool s_sdcard_mounted;

static bool mount_sdcard(const board_config_t *board)
{
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {0};
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_err_t error;

    if (s_sdcard_mounted)
    {
        return true;
    }

    if (board == NULL)
    {
        return false;
    }

    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;
    mount_config.allocation_unit_size = 16 * 1024;

    slot_config.width = board->sdcard.width;
    slot_config.clk = (gpio_num_t)board->sdcard.clk_gpio;
    slot_config.cmd = (gpio_num_t)board->sdcard.cmd_gpio;
    slot_config.d0 = (gpio_num_t)board->sdcard.d0_gpio;

    ESP_LOGI(TAG,
             "mounting sdcard clk=%d cmd=%d d0=%d width=%d",
             board->sdcard.clk_gpio,
             board->sdcard.cmd_gpio,
             board->sdcard.d0_gpio,
             board->sdcard.width);

    error = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &s_sdcard);
    if (error != ESP_OK)
    {
        ESP_LOGW(TAG, "sdcard mount failed err=%s", esp_err_to_name(error));
        s_sdcard = NULL;
        s_sdcard_mounted = false;
        return false;
    }

    s_sdcard_mounted = true;
    sdmmc_card_print_info(stdout, s_sdcard);
    ESP_LOGI(TAG, "sdcard mounted at /sdcard");
    return true;
}

static void unmount_sdcard(void)
{
    if (!s_sdcard_mounted || s_sdcard == NULL)
    {
        return;
    }

    if (esp_vfs_fat_sdcard_unmount("/sdcard", s_sdcard) == ESP_OK)
    {
        ESP_LOGI(TAG, "sdcard unmounted");
    }
    else
    {
        ESP_LOGW(TAG, "sdcard unmount failed");
    }

    s_sdcard = NULL;
    s_sdcard_mounted = false;
}

static provider_type_t provider_type_from_string(const char *value)
{
    if (value == NULL)
    {
        return PROVIDER_TYPE_UNKNOWN;
    }

    if (strcmp(value, "minimax") == 0)
    {
        return PROVIDER_TYPE_MINIMAX;
    }

    return PROVIDER_TYPE_UNKNOWN;
}

static bool parse_hhmm_minutes(const char *value, bool allow_24h, uint16_t *minutes)
{
    int hour;
    int minute;

    if (value == NULL || minutes == NULL || strlen(value) != 5 || value[2] != ':')
    {
        return false;
    }

    if (value[0] < '0' || value[0] > '9' ||
        value[1] < '0' || value[1] > '9' ||
        value[3] < '0' || value[3] > '9' ||
        value[4] < '0' || value[4] > '9')
    {
        return false;
    }

    hour = ((value[0] - '0') * 10) + (value[1] - '0');
    minute = ((value[3] - '0') * 10) + (value[4] - '0');

    if (minute < 0 || minute > 59)
    {
        return false;
    }

    if (hour == 24)
    {
        if (!allow_24h || minute != 0)
        {
            return false;
        }

        *minutes = (uint16_t)(24U * 60U);
        return true;
    }

    if (hour < 0 || hour > 23)
    {
        return false;
    }

    *minutes = (uint16_t)((hour * 60) + minute);
    return true;
}

static bool read_text_file(const char *path, char *buffer, size_t buffer_len)
{
    FILE *file;
    size_t read_len;

    if (path == NULL || buffer == NULL || buffer_len == 0)
    {
        return false;
    }

    file = fopen(path, "rb");
    if (file == NULL)
    {
        return false;
    }

    read_len = fread(buffer, 1, buffer_len - 1, file);
    buffer[read_len] = '\0';
    fclose(file);

    return read_len > 0;
}

static bool parse_provider_item(const cJSON *provider_item, provider_config_t *provider)
{
    const cJSON *id;
    const cJSON *type;
    const cJSON *enabled;
    const cJSON *region;
    const cJSON *api_key;

    if (provider_item == NULL || provider == NULL)
    {
        return false;
    }

    id = cJSON_GetObjectItemCaseSensitive(provider_item, "id");
    type = cJSON_GetObjectItemCaseSensitive(provider_item, "type");
    enabled = cJSON_GetObjectItemCaseSensitive(provider_item, "enabled");
    region = cJSON_GetObjectItemCaseSensitive(provider_item, "region");
    api_key = cJSON_GetObjectItemCaseSensitive(provider_item, "api_key");

    if (!cJSON_IsString(id) || id->valuestring == NULL ||
        !cJSON_IsString(type) || type->valuestring == NULL)
    {
        return false;
    }

    memset(provider, 0, sizeof(*provider));
    strncpy(provider->id, id->valuestring, sizeof(provider->id) - 1);
    provider->provider_type = provider_type_from_string(type->valuestring);
    provider->enabled = cJSON_IsBool(enabled) ? cJSON_IsTrue(enabled) : false;

    if (cJSON_IsString(region) && region->valuestring != NULL)
    {
        strncpy(provider->region, region->valuestring, sizeof(provider->region) - 1);
    }
    if (cJSON_IsString(api_key) && api_key->valuestring != NULL)
    {
        strncpy(provider->api_key, api_key->valuestring, sizeof(provider->api_key) - 1);
    }

    return true;
}

config_store_load_result_t config_store_resolve_precedence(bool sd_valid,
                                                           const char *sd_json,
                                                           bool nvs_valid,
                                                           const char *nvs_json,
                                                           bool *persist_sd_to_nvs)
{
    if (persist_sd_to_nvs != NULL)
    {
        *persist_sd_to_nvs = false;
    }

    if (sd_valid)
    {
        if (!nvs_valid)
        {
            if (persist_sd_to_nvs != NULL)
            {
                *persist_sd_to_nvs = true;
            }
            return CONFIG_STORE_LOAD_FROM_SD;
        }

        if (sd_json != NULL && nvs_json != NULL && strcmp(sd_json, nvs_json) != 0)
        {
            if (persist_sd_to_nvs != NULL)
            {
                *persist_sd_to_nvs = true;
            }
            return CONFIG_STORE_LOAD_SD_OVERRIDES_NVS;
        }

        return CONFIG_STORE_LOAD_FROM_SD;
    }

    if (nvs_valid)
    {
        return CONFIG_STORE_LOAD_FROM_NVS;
    }

    return CONFIG_STORE_LOAD_UNPROVISIONED;
}

bool config_store_parse_json_text(const char *json_text, app_config_t *config)
{
    cJSON *root;
    const cJSON *version;
    const cJSON *device;
    const cJSON *wifi;
    const cJSON *display;
    const cJSON *weather;
    const cJSON *providers;
    const cJSON *item;
    size_t provider_count = 0;

    if (json_text == NULL)
    {
        return false;
    }

    if (config == NULL)
    {
        app_config_t scratch;
        return config_store_parse_json_text(json_text, &scratch);
    }

    app_config_init(config);

    root = cJSON_Parse(json_text);
    if (root == NULL)
    {
        return false;
    }

    version = cJSON_GetObjectItemCaseSensitive(root, "version");
    if (cJSON_IsNumber(version))
    {
        config->version = (uint32_t)version->valuedouble;
    }

    device = cJSON_GetObjectItemCaseSensitive(root, "device");
    if (cJSON_IsObject(device))
    {
        const cJSON *timezone = cJSON_GetObjectItemCaseSensitive(device, "timezone");
        const cJSON *ntp_enabled = cJSON_GetObjectItemCaseSensitive(device, "ntp_enabled");
        const cJSON *ntp_sync_hours = cJSON_GetObjectItemCaseSensitive(device, "ntp_sync_hours");
        const cJSON *sleep_schedule = cJSON_GetObjectItemCaseSensitive(device, "sleep_schedule");

        if (cJSON_IsString(timezone) && timezone->valuestring != NULL)
        {
            strncpy(config->device.timezone, timezone->valuestring, sizeof(config->device.timezone) - 1);
        }
        if (cJSON_IsBool(ntp_enabled))
        {
            config->device.ntp_enabled = cJSON_IsTrue(ntp_enabled);
        }
        if (cJSON_IsNumber(ntp_sync_hours) && ntp_sync_hours->valuedouble > 0)
        {
            config->device.ntp_sync_hours = (uint16_t)ntp_sync_hours->valuedouble;
        }

        if (cJSON_IsObject(sleep_schedule))
        {
            const cJSON *enabled = cJSON_GetObjectItemCaseSensitive(sleep_schedule, "enabled");
            const cJSON *wake_time = cJSON_GetObjectItemCaseSensitive(sleep_schedule, "wake_time");
            const cJSON *sleep_time = cJSON_GetObjectItemCaseSensitive(sleep_schedule, "sleep_time");
            const cJSON *manual_wake_minutes = cJSON_GetObjectItemCaseSensitive(sleep_schedule, "manual_wake_minutes");

            if (cJSON_IsBool(enabled))
            {
                config->device.sleep_schedule.enabled = cJSON_IsTrue(enabled);
            }

            if (cJSON_IsString(wake_time) && wake_time->valuestring != NULL)
            {
                if (!parse_hhmm_minutes(wake_time->valuestring,
                                        false,
                                        &config->device.sleep_schedule.wake_minutes))
                {
                    cJSON_Delete(root);
                    return false;
                }
            }

            if (cJSON_IsString(sleep_time) && sleep_time->valuestring != NULL)
            {
                if (!parse_hhmm_minutes(sleep_time->valuestring,
                                        true,
                                        &config->device.sleep_schedule.sleep_minutes))
                {
                    cJSON_Delete(root);
                    return false;
                }
            }

            if (manual_wake_minutes != NULL)
            {
                if (!cJSON_IsNumber(manual_wake_minutes) || manual_wake_minutes->valuedouble < 0)
                {
                    cJSON_Delete(root);
                    return false;
                }

                config->device.sleep_schedule.manual_wake_minutes = (uint16_t)manual_wake_minutes->valuedouble;
            }
        }
    }

    wifi = cJSON_GetObjectItemCaseSensitive(root, "wifi");
    if (cJSON_IsObject(wifi))
    {
        const cJSON *enabled = cJSON_GetObjectItemCaseSensitive(wifi, "enabled");
        const cJSON *ssid = cJSON_GetObjectItemCaseSensitive(wifi, "ssid");
        const cJSON *password = cJSON_GetObjectItemCaseSensitive(wifi, "password");

        if (cJSON_IsString(ssid) && ssid->valuestring != NULL)
        {
            strncpy(config->wifi.ssid, ssid->valuestring, sizeof(config->wifi.ssid) - 1);
        }
        if (cJSON_IsString(password) && password->valuestring != NULL)
        {
            strncpy(config->wifi.password, password->valuestring, sizeof(config->wifi.password) - 1);
        }
        if (cJSON_IsBool(enabled))
        {
            config->wifi.enabled = cJSON_IsTrue(enabled);
        }
        else if (config->wifi.ssid[0] != '\0')
        {
            config->wifi.enabled = true;
        }
    }

    display = cJSON_GetObjectItemCaseSensitive(root, "display");
    if (cJSON_IsObject(display))
    {
        const cJSON *active_provider_id = cJSON_GetObjectItemCaseSensitive(display, "active_provider_id");
        const cJSON *weather_enabled = cJSON_GetObjectItemCaseSensitive(display, "weather_enabled");

        if (cJSON_IsString(active_provider_id) && active_provider_id->valuestring != NULL)
        {
            strncpy(config->display.active_provider_id,
                    active_provider_id->valuestring,
                    sizeof(config->display.active_provider_id) - 1);
        }
        if (cJSON_IsBool(weather_enabled))
        {
            config->display.weather_enabled = cJSON_IsTrue(weather_enabled);
        }
    }

    weather = cJSON_GetObjectItemCaseSensitive(root, "weather");
    if (cJSON_IsObject(weather))
    {
        const cJSON *enabled = cJSON_GetObjectItemCaseSensitive(weather, "enabled");
        if (cJSON_IsBool(enabled))
        {
            config->weather.enabled = cJSON_IsTrue(enabled);
        }
    }

    providers = cJSON_GetObjectItemCaseSensitive(root, "providers");
    if (cJSON_IsArray(providers))
    {
        cJSON_ArrayForEach(item, providers)
        {
            if (provider_count >= APP_CONFIG_MAX_PROVIDERS)
            {
                break;
            }

            if (!parse_provider_item(item, &config->providers[provider_count]))
            {
                cJSON_Delete(root);
                return false;
            }

            ++provider_count;
        }
    }
    config->provider_count = provider_count;

    cJSON_Delete(root);
    return app_config_validate(config);
}

static bool nvs_init_default_partition(void)
{
    esp_err_t error = nvs_flash_init();
    if (error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        if (nvs_flash_erase() != ESP_OK)
        {
            return false;
        }
        error = nvs_flash_init();
    }

    return error == ESP_OK;
}

static bool read_nvs_json(char *buffer, size_t buffer_len)
{
    nvs_handle_t handle;
    size_t required_len = buffer_len;
    esp_err_t error;

    if (buffer == NULL || buffer_len == 0)
    {
        return false;
    }

    if (!nvs_init_default_partition())
    {
        return false;
    }

    error = nvs_open(CONFIG_NAMESPACE, NVS_READONLY, &handle);
    if (error != ESP_OK)
    {
        return false;
    }

    error = nvs_get_str(handle, CONFIG_KEY, buffer, &required_len);
    nvs_close(handle);
    return error == ESP_OK && required_len > 0;
}

bool config_store_persist_json_text(const char *json_text)
{
    nvs_handle_t handle;
    esp_err_t error;

    if (json_text == NULL || json_text[0] == '\0')
    {
        return false;
    }

    if (!nvs_init_default_partition())
    {
        return false;
    }

    error = nvs_open(CONFIG_NAMESPACE, NVS_READWRITE, &handle);
    if (error != ESP_OK)
    {
        return false;
    }

    error = nvs_set_str(handle, CONFIG_KEY, json_text);
    if (error == ESP_OK)
    {
        error = nvs_commit(handle);
    }
    nvs_close(handle);

    return error == ESP_OK;
}

bool config_store_import_json_text(const char *json_text, config_source_t source, app_config_t *config)
{
    app_config_t parsed;

    if (!config_store_parse_json_text(json_text, &parsed))
    {
        return false;
    }

    if (!config_store_persist_json_text(json_text))
    {
        return false;
    }

    parsed.source = source;
    if (config != NULL)
    {
        *config = parsed;
    }

    return true;
}

void config_store_init(const board_config_t *board)
{
    (void)mount_sdcard(board);
}

void config_store_release_transient_resources(void)
{
    unmount_sdcard();
}

bool config_store_is_provisioned(const app_config_t *config)
{
    const provider_config_t *active_provider;

    if (config == NULL)
    {
        return false;
    }

    active_provider = app_config_find_active_provider(config);
    return active_provider != NULL && active_provider->api_key[0] != '\0';
}

bool config_store_load_effective(app_config_t *config, config_store_state_t *state)
{
    char *sd_json;
    char *nvs_json;
    app_config_t sd_config;
    app_config_t nvs_config;
    bool sd_valid = false;
    bool nvs_valid = false;
    bool persist_sd_to_nvs = false;
    config_store_load_result_t resolved_result;

    if (config == NULL)
    {
        return false;
    }

    app_config_init(config);

    if (state != NULL)
    {
        state->sd_available = false;
        state->nvs_available = false;
        state->result = CONFIG_STORE_LOAD_UNPROVISIONED;
    }

    config->source = CONFIG_SOURCE_NONE;
    config->provider_count = 1;
    strncpy(config->providers[0].id, "minimax-cn", sizeof(config->providers[0].id) - 1);
    config->providers[0].provider_type = PROVIDER_TYPE_MINIMAX;
    config->providers[0].enabled = false;
    strncpy(config->providers[0].region, "cn", sizeof(config->providers[0].region) - 1);
    config->display.active_provider_id[0] = '\0';

    sd_json = malloc(CONFIG_JSON_MAX_LEN);
    nvs_json = malloc(CONFIG_JSON_MAX_LEN);

    if (sd_json != NULL)
    {
        sd_valid = read_text_file(CONFIG_SD_PATH, sd_json, CONFIG_JSON_MAX_LEN) && config_store_parse_json_text(sd_json, &sd_config);
    }

    if (nvs_json != NULL)
    {
        nvs_valid = read_nvs_json(nvs_json, CONFIG_JSON_MAX_LEN) && config_store_parse_json_text(nvs_json, &nvs_config);
    }

    if (state != NULL)
    {
        state->sd_available = sd_valid;
        state->nvs_available = nvs_valid;
    }

    resolved_result = config_store_resolve_precedence(sd_valid,
                                                      sd_json,
                                                      nvs_valid,
                                                      nvs_json,
                                                      &persist_sd_to_nvs);

    if (resolved_result == CONFIG_STORE_LOAD_FROM_SD || resolved_result == CONFIG_STORE_LOAD_SD_OVERRIDES_NVS)
    {
        *config = sd_config;
        config->source = CONFIG_SOURCE_SD_CARD;

        if (persist_sd_to_nvs && sd_json != NULL)
        {
            (void)config_store_persist_json_text(sd_json);
        }

        if (state != NULL)
        {
            state->result = resolved_result;
        }

        free(sd_json);
        free(nvs_json);
        return true;
    }

    if (resolved_result == CONFIG_STORE_LOAD_FROM_NVS)
    {
        *config = nvs_config;
        config->source = CONFIG_SOURCE_NVS;
        if (state != NULL)
        {
            state->result = resolved_result;
        }

        free(sd_json);
        free(nvs_json);
        return true;
    }

    free(sd_json);
    free(nvs_json);
    return app_config_validate(config);
}