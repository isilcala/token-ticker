#include "board.h"

static const board_config_t k_board_config = {
    .name = "waveshare-s3-rlcd-4.2",
    .i2c = {
        .sda_gpio = 13,
        .scl_gpio = 14,
    },
    .display = {
        .dc_gpio = 5,
        .cs_gpio = 40,
        .sck_gpio = 11,
        .mosi_gpio = 12,
        .rst_gpio = 41,
        .te_gpio = 6,
        .native_width = 300,
        .native_height = 400,
        .display_width = 400,
        .display_height = 300,
        .landscape = true,
    },
    .buttons = {
        .boot_gpio = 0,
        .user_gpio = 18,
        .active_low = true,
    },
    .power = {
        .battery_adc_gpio = 4,
        .battery_adc_channel = 3,
        .battery_divider_ratio = 3.0f,
        .battery_empty_volts = 3.0f,
        .battery_full_volts = 4.2f,
        .charge_state_readable = false,
    },
    .sdcard = {
        .clk_gpio = 38,
        .cmd_gpio = 21,
        .d0_gpio = 39,
        .width = 1,
    },
};

const board_config_t *board_get_config(void)
{
    return &k_board_config;
}