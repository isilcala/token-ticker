#ifndef TOKEN_TICKER_HTTP_CLIENT_H
#define TOKEN_TICKER_HTTP_CLIENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HTTP_URL_MAX_LEN 192
#define HTTP_HEADER_VALUE_MAX_LEN 160

typedef enum
{
    TOKEN_HTTP_METHOD_GET = 0,
} http_method_t;

typedef enum
{
    HTTP_CLIENT_STATUS_IDLE = 0,
    HTTP_CLIENT_STATUS_NOT_IMPLEMENTED,
    HTTP_CLIENT_STATUS_TRANSPORT_ERROR,
    HTTP_CLIENT_STATUS_HTTP_ERROR,
    HTTP_CLIENT_STATUS_BUFFER_TOO_SMALL,
    HTTP_CLIENT_STATUS_OK,
} http_client_status_t;

typedef struct
{
    http_method_t method;
    char url[HTTP_URL_MAX_LEN];
    char bearer_token[HTTP_HEADER_VALUE_MAX_LEN];
    uint32_t timeout_ms;
} http_request_t;

typedef struct
{
    http_client_status_t status;
    uint16_t http_status_code;
    size_t bytes_received;
} http_response_meta_t;

void http_request_init(http_request_t *request);
bool http_request_set_url(http_request_t *request, const char *url);
bool http_request_set_bearer_token(http_request_t *request, const char *token);
bool http_client_get_json(const http_request_t *request,
                          char *response_buffer,
                          size_t response_buffer_len,
                          http_response_meta_t *meta);

#endif