#ifndef TOKEN_TICKER_SENSOR_SERVICE_H
#define TOKEN_TICKER_SENSOR_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "board.h"

typedef struct
{
    int16_t temperature_c_x10;
    int16_t humidity_rh_x10;
    bool valid;
} environment_sample_t;

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool valid;
} rtc_time_t;

void sensor_service_init(const board_config_t *board);
bool sensor_service_read_environment(environment_sample_t *sample);
bool sensor_service_read_rtc(rtc_time_t *time_value);
bool sensor_service_write_rtc(const rtc_time_t *time_value);

#endif