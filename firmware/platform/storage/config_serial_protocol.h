#ifndef TOKEN_TICKER_CONFIG_SERIAL_PROTOCOL_H
#define TOKEN_TICKER_CONFIG_SERIAL_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>

#include "app_config.h"

typedef enum
{
    CONFIG_SERIAL_RESULT_IGNORED = 0,
    CONFIG_SERIAL_RESULT_OK,
    CONFIG_SERIAL_RESULT_EMPTY,
    CONFIG_SERIAL_RESULT_INVALID_FORMAT,
    CONFIG_SERIAL_RESULT_INVALID_JSON,
    CONFIG_SERIAL_RESULT_PERSIST_FAILED,
} config_serial_result_t;

bool config_serial_protocol_apply_line(const char *line,
                                       app_config_t *config,
                                       config_serial_result_t *result,
                                       char *message,
                                       size_t message_len);

const char *config_serial_result_text(config_serial_result_t result);

#endif