#ifndef TOKEN_TICKER_CONFIG_STORE_H
#define TOKEN_TICKER_CONFIG_STORE_H

#include <stdbool.h>

#include "app_config.h"
#include "board.h"

typedef enum
{
    CONFIG_STORE_LOAD_ERROR = 0,
    CONFIG_STORE_LOAD_UNPROVISIONED,
    CONFIG_STORE_LOAD_FROM_SD,
    CONFIG_STORE_LOAD_FROM_NVS,
    CONFIG_STORE_LOAD_SD_OVERRIDES_NVS,
} config_store_load_result_t;

typedef struct
{
    bool sd_available;
    bool nvs_available;
    config_store_load_result_t result;
} config_store_state_t;

config_store_load_result_t config_store_resolve_precedence(bool sd_valid,
                                                           const char *sd_json,
                                                           bool nvs_valid,
                                                           const char *nvs_json,
                                                           bool *persist_sd_to_nvs);
void config_store_init(const board_config_t *board);
void config_store_release_transient_resources(void);
bool config_store_is_provisioned(const app_config_t *config);
bool config_store_parse_json_text(const char *json_text, app_config_t *config);
bool config_store_persist_json_text(const char *json_text);
bool config_store_import_json_text(const char *json_text, config_source_t source, app_config_t *config);
bool config_store_load_effective(app_config_t *config, config_store_state_t *state);

#endif