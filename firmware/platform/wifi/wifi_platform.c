#include "wifi_platform.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "wifi_platform";
static bool s_netif_ready;
static bool s_event_loop_ready;
static bool s_handlers_registered;
static bool s_wifi_driver_ready;
static bool s_wifi_started;
static bool s_wifi_connected;
static bool s_wifi_reconnect_enabled;
static esp_netif_t *s_sta_netif;
static char s_status_text[32] = "wifi_unconfigured";

static void wifi_platform_set_status(const char *status)
{
    if (status == NULL)
    {
        return;
    }

    snprintf(s_status_text, sizeof(s_status_text), "%s", status);
}

static bool wifi_platform_has_credentials(const app_config_t *config)
{
    return config != NULL && config->wifi.enabled && config->wifi.ssid[0] != '\0';
}

static void wifi_platform_event_handler(void *arg,
                                        esp_event_base_t event_base,
                                        int32_t event_id,
                                        void *event_data)
{
    (void)arg;

    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            s_wifi_connected = false;
            wifi_platform_set_status("wifi_connecting");
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            s_wifi_connected = false;
            if (s_wifi_reconnect_enabled)
            {
                wifi_platform_set_status("wifi_connecting");
                ESP_LOGW(TAG, "wifi disconnected, reconnecting");
                ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
            }
            else
            {
                wifi_platform_set_status("wifi_stopped");
            }
        }
        else if (event_id == WIFI_EVENT_STA_STOP)
        {
            s_wifi_connected = false;
            s_wifi_started = false;
            wifi_platform_set_status("wifi_stopped");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        s_wifi_connected = true;
        wifi_platform_set_status("wifi_connected");
        ESP_LOGI(TAG, "wifi connected");
    }
}

bool wifi_platform_start(const app_config_t *config)
{
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config = {0};
    esp_err_t error;

    if (!wifi_platform_has_credentials(config))
    {
        s_wifi_connected = false;
        wifi_platform_set_status("wifi_unconfigured");
        return false;
    }

    if (!s_netif_ready)
    {
        error = esp_netif_init();
        if (error != ESP_OK && error != ESP_ERR_INVALID_STATE)
        {
            wifi_platform_set_status("wifi_stack_error");
            ESP_LOGE(TAG, "esp_netif_init failed err=%s", esp_err_to_name(error));
            return false;
        }
        s_netif_ready = true;
    }

    if (!s_event_loop_ready)
    {
        error = esp_event_loop_create_default();
        if (error != ESP_OK && error != ESP_ERR_INVALID_STATE)
        {
            wifi_platform_set_status("wifi_event_error");
            ESP_LOGE(TAG, "esp_event_loop_create_default failed err=%s", esp_err_to_name(error));
            return false;
        }
        s_event_loop_ready = true;
    }

    if (s_sta_netif == NULL)
    {
        s_sta_netif = esp_netif_create_default_wifi_sta();
        if (s_sta_netif == NULL)
        {
            wifi_platform_set_status("wifi_netif_error");
            ESP_LOGE(TAG, "esp_netif_create_default_wifi_sta failed");
            return false;
        }
    }

    if (!s_wifi_driver_ready)
    {
        error = esp_wifi_init(&init_config);
        if (error != ESP_OK)
        {
            wifi_platform_set_status("wifi_init_error");
            ESP_LOGE(TAG, "esp_wifi_init failed err=%s", esp_err_to_name(error));
            return false;
        }

        error = esp_wifi_set_storage(WIFI_STORAGE_RAM);
        if (error != ESP_OK)
        {
            wifi_platform_set_status("wifi_storage_error");
            ESP_LOGE(TAG, "esp_wifi_set_storage failed err=%s", esp_err_to_name(error));
            return false;
        }

        s_wifi_driver_ready = true;
    }

    if (!s_handlers_registered)
    {
        error = esp_event_handler_register(WIFI_EVENT,
                                           ESP_EVENT_ANY_ID,
                                           &wifi_platform_event_handler,
                                           NULL);
        if (error != ESP_OK)
        {
            wifi_platform_set_status("wifi_handler_error");
            ESP_LOGE(TAG, "wifi event handler register failed err=%s", esp_err_to_name(error));
            return false;
        }

        error = esp_event_handler_register(IP_EVENT,
                                           IP_EVENT_STA_GOT_IP,
                                           &wifi_platform_event_handler,
                                           NULL);
        if (error != ESP_OK)
        {
            wifi_platform_set_status("wifi_handler_error");
            ESP_LOGE(TAG, "ip event handler register failed err=%s", esp_err_to_name(error));
            return false;
        }

        s_handlers_registered = true;
    }

    strncpy((char *)wifi_config.sta.ssid, config->wifi.ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, config->wifi.password, sizeof(wifi_config.sta.password) - 1);

    error = esp_wifi_set_mode(WIFI_MODE_STA);
    if (error != ESP_OK)
    {
        wifi_platform_set_status("wifi_mode_error");
        ESP_LOGE(TAG, "esp_wifi_set_mode failed err=%s", esp_err_to_name(error));
        return false;
    }

    error = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (error != ESP_OK)
    {
        wifi_platform_set_status("wifi_config_error");
        ESP_LOGE(TAG, "esp_wifi_set_config failed err=%s", esp_err_to_name(error));
        return false;
    }

    s_wifi_connected = false;
    s_wifi_reconnect_enabled = true;
    wifi_platform_set_status("wifi_connecting");

    if (!s_wifi_started)
    {
        error = esp_wifi_start();
        if (error != ESP_OK)
        {
            wifi_platform_set_status("wifi_start_error");
            ESP_LOGE(TAG, "esp_wifi_start failed err=%s", esp_err_to_name(error));
            return false;
        }
        s_wifi_started = true;
    }
    else
    {
        (void)esp_wifi_disconnect();
        error = esp_wifi_connect();
        if (error != ESP_OK)
        {
            wifi_platform_set_status("wifi_connect_error");
            ESP_LOGE(TAG, "esp_wifi_connect failed err=%s", esp_err_to_name(error));
            return false;
        }
    }

    ESP_LOGI(TAG, "wifi start requested ssid=%s", config->wifi.ssid);
    return true;
}

bool wifi_platform_ensure_ready(const app_config_t *config, uint32_t timeout_ms)
{
    if (wifi_platform_is_ready())
    {
        return true;
    }

    if (!wifi_platform_start(config))
    {
        return false;
    }

    return wifi_platform_wait_ready(timeout_ms);
}

bool wifi_platform_wait_ready(uint32_t timeout_ms)
{
    uint32_t waited_ms = 0;

    while (!s_wifi_connected && waited_ms < timeout_ms)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        waited_ms += 100;
    }

    if (!s_wifi_connected && timeout_ms > 0)
    {
        wifi_platform_set_status("wifi_timeout");
        ESP_LOGW(TAG, "wifi wait timed out status=%s", s_status_text);
    }

    return s_wifi_connected;
}

bool wifi_platform_stop(void)
{
    esp_err_t error;

    s_wifi_reconnect_enabled = false;
    s_wifi_connected = false;

    if (!s_wifi_driver_ready)
    {
        wifi_platform_set_status("wifi_stopped");
        return true;
    }

    if (!s_wifi_started)
    {
        wifi_platform_set_status("wifi_stopped");
        return true;
    }

    (void)esp_wifi_disconnect();
    error = esp_wifi_stop();
    if (error != ESP_OK && error != ESP_ERR_WIFI_NOT_INIT && error != ESP_ERR_WIFI_NOT_STARTED)
    {
        ESP_LOGW(TAG, "esp_wifi_stop failed err=%s", esp_err_to_name(error));
        return false;
    }

    s_wifi_started = false;
    wifi_platform_set_status("wifi_stopped");
    ESP_LOGI(TAG, "wifi stopped");
    return true;
}

bool wifi_platform_is_ready(void)
{
    return s_wifi_connected;
}

bool wifi_platform_get_rssi_dbm(int8_t *rssi_dbm)
{
    wifi_ap_record_t ap_info;

    if (rssi_dbm == NULL || !s_wifi_connected)
    {
        return false;
    }

    memset(&ap_info, 0, sizeof(ap_info));
    if (esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK)
    {
        return false;
    }

    *rssi_dbm = ap_info.rssi;
    return true;
}

const char *wifi_platform_status_text(void)
{
    return s_status_text;
}