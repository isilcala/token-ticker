#include "http_client.h"

#include <stdio.h>
#include <string.h>

#include "esp_crt_bundle.h"
#include "esp_err.h"
#include "esp_http_client.h"

static esp_http_client_method_t http_method_to_esp(http_method_t method)
{
    switch (method)
    {
    case TOKEN_HTTP_METHOD_GET:
    default:
        return HTTP_METHOD_GET;
    }
}

void http_request_init(http_request_t *request)
{
    if (request == NULL)
    {
        return;
    }

    memset(request, 0, sizeof(*request));
    request->method = TOKEN_HTTP_METHOD_GET;
    request->timeout_ms = 10000;
}

bool http_request_set_url(http_request_t *request, const char *url)
{
    if (request == NULL || url == NULL)
    {
        return false;
    }

    if (strlen(url) >= sizeof(request->url))
    {
        return false;
    }

    strcpy(request->url, url);
    return true;
}

bool http_request_set_bearer_token(http_request_t *request, const char *token)
{
    if (request == NULL || token == NULL)
    {
        return false;
    }

    if (strlen(token) >= sizeof(request->bearer_token))
    {
        return false;
    }

    strcpy(request->bearer_token, token);
    return true;
}

bool http_client_get_json(const http_request_t *request,
                          char *response_buffer,
                          size_t response_buffer_len,
                          http_response_meta_t *meta)
{
    esp_http_client_config_t config;
    esp_http_client_handle_t client;
    esp_err_t error;
    int read_len;
    size_t total_read = 0;
    char auth_header[HTTP_HEADER_VALUE_MAX_LEN + 8];

    if (meta != NULL)
    {
        meta->status = HTTP_CLIENT_STATUS_NOT_IMPLEMENTED;
        meta->http_status_code = 0;
        meta->bytes_received = 0;
    }

    if (request == NULL || response_buffer == NULL || response_buffer_len == 0)
    {
        return false;
    }

    response_buffer[0] = '\0';

    memset(&config, 0, sizeof(config));
    config.url = request->url;
    config.method = http_method_to_esp(request->method);
    config.timeout_ms = (int)request->timeout_ms;
    config.crt_bundle_attach = esp_crt_bundle_attach;

    client = esp_http_client_init(&config);
    if (client == NULL)
    {
        if (meta != NULL)
        {
            meta->status = HTTP_CLIENT_STATUS_TRANSPORT_ERROR;
        }
        return false;
    }

    if (request->bearer_token[0] != '\0')
    {
        snprintf(auth_header, sizeof(auth_header), "Bearer %s", request->bearer_token);
        esp_http_client_set_header(client, "Authorization", auth_header);
    }

    esp_http_client_set_header(client, "Accept", "application/json");

    error = esp_http_client_open(client, 0);
    if (error != ESP_OK)
    {
        if (meta != NULL)
        {
            meta->status = HTTP_CLIENT_STATUS_TRANSPORT_ERROR;
        }
        esp_http_client_cleanup(client);
        return false;
    }

    (void)esp_http_client_fetch_headers(client);

    if (meta != NULL)
    {
        meta->http_status_code = (uint16_t)esp_http_client_get_status_code(client);
    }

    while ((read_len = esp_http_client_read(client,
                                            response_buffer + total_read,
                                            (int)(response_buffer_len - 1 - total_read))) > 0)
    {
        total_read += (size_t)read_len;
        if (total_read >= response_buffer_len - 1)
        {
            response_buffer[response_buffer_len - 1] = '\0';
            if (meta != NULL)
            {
                meta->status = HTTP_CLIENT_STATUS_BUFFER_TOO_SMALL;
                meta->bytes_received = total_read;
            }
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return false;
        }
    }

    response_buffer[total_read] = '\0';

    if (meta != NULL)
    {
        meta->bytes_received = total_read;
    }

    if (read_len < 0)
    {
        if (meta != NULL)
        {
            meta->status = HTTP_CLIENT_STATUS_TRANSPORT_ERROR;
        }
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return false;
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (meta != NULL)
    {
        meta->status = (meta->http_status_code >= 200 && meta->http_status_code < 300)
                           ? HTTP_CLIENT_STATUS_OK
                           : HTTP_CLIENT_STATUS_HTTP_ERROR;
    }

    return meta == NULL || meta->status == HTTP_CLIENT_STATUS_OK;
}