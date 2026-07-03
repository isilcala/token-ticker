#ifndef TOKEN_TICKER_MINIMAX_HTTP_H
#define TOKEN_TICKER_MINIMAX_HTTP_H

#include <stdbool.h>

#include "app_config.h"
#include "http_client.h"
#include "minimax_quota_types.h"

bool minimax_http_build_quota_request(const provider_config_t *config, http_request_t *request);
bool minimax_http_parse_quota_response_json(const char *json_text, minimax_quota_response_t *response);

#endif