#include "esp_log.h"

#include "app_bootstrap.h"
#include "app_runtime.h"
#include "board.h"
#include "ui_app.h"

static const char *TAG = "token_ticker";
static app_bootstrap_context_t s_bootstrap_context;

void app_main(void)
{
    const board_config_t *board = board_get_config();

    ESP_LOGI(TAG, "booting board=%s display=%ux%u", board->name, board->display.display_width, board->display.display_height);
    ui_app_show_boot_status(board, "BOOTING", "starting device", 2);

    if (!app_bootstrap_run(board, &s_bootstrap_context))
    {
        ESP_LOGE(TAG, "bootstrap failed");
        return;
    }

    ui_app_boot(&s_bootstrap_context.ui_boot_model);
    app_bootstrap_enable_automatic_light_sleep(&s_bootstrap_context);

    if (!app_runtime_start(&s_bootstrap_context))
    {
        ESP_LOGE(TAG, "runtime start failed");
    }
}