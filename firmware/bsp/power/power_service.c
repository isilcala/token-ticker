#include "power_service.h"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"

static const board_config_t *s_board_config;
static adc_oneshot_unit_handle_t s_adc_unit;
static adc_cali_handle_t s_adc_cali;
static bool s_adc_ready;
static bool s_adc_calibrated;

static const char *TAG = "power_service";

static esp_err_t power_service_create_calibration(const board_config_t *board)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = (adc_channel_t)board->power.battery_adc_channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    return adc_cali_create_scheme_curve_fitting(&cali_config, &s_adc_cali);
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    return adc_cali_create_scheme_line_fitting(&cali_config, &s_adc_cali);
#else
    (void)board;
    return ESP_ERR_NOT_SUPPORTED;
#endif
}

void power_service_init(const board_config_t *board)
{
    esp_err_t error;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    s_board_config = board;
    s_adc_ready = false;
    s_adc_calibrated = false;
    s_adc_unit = NULL;
    s_adc_cali = NULL;

    if (board == NULL)
    {
        return;
    }

    error = adc_oneshot_new_unit(&init_config, &s_adc_unit);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "adc unit init failed err=%d", (int)error);
        return;
    }

    error = adc_oneshot_config_channel(s_adc_unit, (adc_channel_t)board->power.battery_adc_channel, &channel_config);
    if (error != ESP_OK)
    {
        ESP_LOGE(TAG, "adc channel config failed err=%d", (int)error);
        return;
    }

    error = power_service_create_calibration(board);
    if (error == ESP_OK)
    {
        s_adc_calibrated = true;
    }
    else
    {
        ESP_LOGW(TAG, "adc calibration unavailable err=%d", (int)error);
    }

    s_adc_ready = true;
}

uint8_t power_service_calculate_percent(const board_config_t *board, int32_t battery_mv)
{
    const float empty_mv = board->power.battery_empty_volts * 1000.0f;
    const float full_mv = board->power.battery_full_volts * 1000.0f;
    const float range_mv = full_mv - empty_mv;
    float scaled;

    if (board == NULL || range_mv <= 0.0f)
    {
        return 0;
    }

    if ((float)battery_mv <= empty_mv)
    {
        return 0;
    }

    if ((float)battery_mv >= full_mv)
    {
        return 100;
    }

    scaled = (((float)battery_mv - empty_mv) / range_mv) * 100.0f;
    if (scaled < 0.0f)
    {
        scaled = 0.0f;
    }
    if (scaled > 100.0f)
    {
        scaled = 100.0f;
    }

    return (uint8_t)(scaled + 0.5f);
}

bool power_service_get_status(power_status_t *status)
{
    int raw_value;
    int sensed_mv;
    esp_err_t error;

    if (status == NULL || s_board_config == NULL)
    {
        return false;
    }

    status->battery_mv = 0;
    status->battery_percent = 0;
    status->charge_state = POWER_CHARGE_STATE_UNKNOWN;
    status->valid = false;

    if (!s_adc_ready || !s_adc_calibrated || s_adc_unit == NULL || s_adc_cali == NULL)
    {
        return false;
    }

    error = adc_oneshot_read(s_adc_unit, (adc_channel_t)s_board_config->power.battery_adc_channel, &raw_value);
    if (error != ESP_OK)
    {
        ESP_LOGW(TAG, "adc read failed err=%d", (int)error);
        return false;
    }

    error = adc_cali_raw_to_voltage(s_adc_cali, raw_value, &sensed_mv);
    if (error != ESP_OK)
    {
        ESP_LOGW(TAG, "adc calibration convert failed err=%d", (int)error);
        return false;
    }

    status->battery_mv = (int32_t)((float)sensed_mv * s_board_config->power.battery_divider_ratio + 0.5f);
    status->battery_percent = power_service_calculate_percent(s_board_config, status->battery_mv);
    status->charge_state = s_board_config->power.charge_state_readable ? POWER_CHARGE_STATE_UNKNOWN : POWER_CHARGE_STATE_UNKNOWN;
    status->valid = true;

    return true;
}