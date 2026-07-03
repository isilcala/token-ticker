#include "config_serial_protocol.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "config_store.h"

static const char *CONFIG_JSON_PREFIX = "CONFIG_JSON ";

static const char *skip_spaces(const char *cursor)
{
    while (cursor != NULL && *cursor != '\0' && isspace((unsigned char)*cursor))
    {
        ++cursor;
    }
    return cursor;
}

const char *config_serial_result_text(config_serial_result_t result)
{
    switch (result)
    {
    case CONFIG_SERIAL_RESULT_OK:
        return "ok";
    case CONFIG_SERIAL_RESULT_EMPTY:
        return "empty";
    case CONFIG_SERIAL_RESULT_INVALID_FORMAT:
        return "invalid_format";
    case CONFIG_SERIAL_RESULT_INVALID_JSON:
        return "invalid_json";
    case CONFIG_SERIAL_RESULT_PERSIST_FAILED:
        return "persist_failed";
    case CONFIG_SERIAL_RESULT_IGNORED:
    default:
        return "ignored";
    }
}

bool config_serial_protocol_apply_line(const char *line,
                                       app_config_t *config,
                                       config_serial_result_t *result,
                                       char *message,
                                       size_t message_len)
{
    const char *payload;

    if (result != NULL)
    {
        *result = CONFIG_SERIAL_RESULT_IGNORED;
    }
    if (message != NULL && message_len > 0)
    {
        message[0] = '\0';
    }

    if (line == NULL)
    {
        return false;
    }

    payload = skip_spaces(line);
    if (*payload == '\0')
    {
        if (result != NULL)
        {
            *result = CONFIG_SERIAL_RESULT_EMPTY;
        }
        if (message != NULL && message_len > 0)
        {
            snprintf(message, message_len, "empty input");
        }
        return false;
    }

    if (strncmp(payload, CONFIG_JSON_PREFIX, strlen(CONFIG_JSON_PREFIX)) == 0)
    {
        payload += strlen(CONFIG_JSON_PREFIX);
        payload = skip_spaces(payload);
    }
    else if (*payload != '{')
    {
        if (result != NULL)
        {
            *result = CONFIG_SERIAL_RESULT_INVALID_FORMAT;
        }
        if (message != NULL && message_len > 0)
        {
            snprintf(message, message_len, "expected raw JSON or CONFIG_JSON <json>");
        }
        return false;
    }

    if (*payload == '\0')
    {
        if (result != NULL)
        {
            *result = CONFIG_SERIAL_RESULT_EMPTY;
        }
        if (message != NULL && message_len > 0)
        {
            snprintf(message, message_len, "missing JSON payload");
        }
        return false;
    }

    if (config_store_import_json_text(payload, CONFIG_SOURCE_SERIAL, config))
    {
        if (result != NULL)
        {
            *result = CONFIG_SERIAL_RESULT_OK;
        }
        if (message != NULL && message_len > 0)
        {
            snprintf(message, message_len, "config imported");
        }
        return true;
    }

    if (!config_store_parse_json_text(payload, NULL))
    {
        if (result != NULL)
        {
            *result = CONFIG_SERIAL_RESULT_INVALID_JSON;
        }
        if (message != NULL && message_len > 0)
        {
            snprintf(message, message_len, "json invalid or fails validation");
        }
        return false;
    }

    if (result != NULL)
    {
        *result = CONFIG_SERIAL_RESULT_PERSIST_FAILED;
    }
    if (message != NULL && message_len > 0)
    {
        snprintf(message, message_len, "json valid but persist failed");
    }
    return false;
}