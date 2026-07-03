#ifndef TOKEN_TICKER_POWER_SERVICE_H
#define TOKEN_TICKER_POWER_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#include "board.h"

typedef enum
{
    POWER_CHARGE_STATE_UNKNOWN = 0,
    POWER_CHARGE_STATE_CHARGING,
    POWER_CHARGE_STATE_DISCHARGING,
    POWER_CHARGE_STATE_FULL,
} power_charge_state_t;

typedef struct
{
    int32_t battery_mv;
    uint8_t battery_percent;
    power_charge_state_t charge_state;
    bool valid;
} power_status_t;

void power_service_init(const board_config_t *board);
bool power_service_get_status(power_status_t *status);
uint8_t power_service_calculate_percent(const board_config_t *board, int32_t battery_mv);

#endif