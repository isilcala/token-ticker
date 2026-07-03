#include "app_key_input.h"

#include <stddef.h>

#include "driver/gpio.h"
#include "esp_err.h"

enum
{
    KEY_DEBOUNCE_TICKS = 200 / portTICK_PERIOD_MS,
};

static TaskHandle_t s_runtime_task;
static volatile bool s_press_pending;
static volatile TickType_t s_last_press_tick;
static bool s_initialized;

static void IRAM_ATTR app_key_input_isr(void *arg)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    TickType_t now_ticks = xTaskGetTickCountFromISR();

    (void)arg;

    if (s_last_press_tick != 0 && (now_ticks - s_last_press_tick) < KEY_DEBOUNCE_TICKS)
    {
        return;
    }

    s_last_press_tick = now_ticks;
    s_press_pending = true;

    if (s_runtime_task != NULL)
    {
        vTaskNotifyGiveFromISR(s_runtime_task, &higher_priority_task_woken);
        if (higher_priority_task_woken == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
    }
}

bool app_key_input_init(const board_config_t *board)
{
    gpio_config_t config = {0};
    gpio_int_type_t intr_type;

    if (board == NULL || board->buttons.user_gpio < 0)
    {
        return false;
    }

    if (s_initialized)
    {
        return true;
    }

    config.pin_bit_mask = 1ULL << board->buttons.user_gpio;
    config.mode = GPIO_MODE_INPUT;
    config.pull_up_en = board->buttons.active_low ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    config.pull_down_en = board->buttons.active_low ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE;
    config.intr_type = board->buttons.active_low ? GPIO_INTR_NEGEDGE : GPIO_INTR_POSEDGE;

    if (gpio_config(&config) != ESP_OK)
    {
        return false;
    }

    esp_err_t error = gpio_install_isr_service(0);
    if (error != ESP_OK && error != ESP_ERR_INVALID_STATE)
    {
        return false;
    }

    intr_type = board->buttons.active_low ? GPIO_INTR_NEGEDGE : GPIO_INTR_POSEDGE;
    if (gpio_set_intr_type((gpio_num_t)board->buttons.user_gpio, intr_type) != ESP_OK)
    {
        return false;
    }

    if (gpio_isr_handler_add((gpio_num_t)board->buttons.user_gpio,
                             app_key_input_isr,
                             NULL) != ESP_OK)
    {
        return false;
    }

    s_press_pending = false;
    s_last_press_tick = 0;
    s_initialized = true;
    return true;
}

void app_key_input_register_task(TaskHandle_t task_handle)
{
    s_runtime_task = task_handle;
}

bool app_key_input_consume_press(void)
{
    bool had_press = s_press_pending;

    s_press_pending = false;
    return had_press;
}