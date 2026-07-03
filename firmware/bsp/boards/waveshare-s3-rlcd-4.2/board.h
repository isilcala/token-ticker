#ifndef TOKEN_TICKER_BOARD_H
#define TOKEN_TICKER_BOARD_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int sda_gpio;
    int scl_gpio;
} board_i2c_bus_t;

typedef struct
{
    int dc_gpio;
    int cs_gpio;
    int sck_gpio;
    int mosi_gpio;
    int rst_gpio;
    int te_gpio;
    uint16_t native_width;
    uint16_t native_height;
    uint16_t display_width;
    uint16_t display_height;
    bool landscape;
} board_display_t;

typedef struct
{
    int boot_gpio;
    int user_gpio;
    bool active_low;
} board_buttons_t;

typedef struct
{
    int battery_adc_gpio;
    int battery_adc_channel;
    float battery_divider_ratio;
    float battery_empty_volts;
    float battery_full_volts;
    bool charge_state_readable;
} board_power_t;

typedef struct
{
    int clk_gpio;
    int cmd_gpio;
    int d0_gpio;
    int width;
} board_sdcard_t;

typedef struct
{
    const char *name;
    board_i2c_bus_t i2c;
    board_display_t display;
    board_buttons_t buttons;
    board_power_t power;
    board_sdcard_t sdcard;
} board_config_t;

const board_config_t *board_get_config(void);

#endif