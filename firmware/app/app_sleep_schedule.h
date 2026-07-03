#ifndef TOKEN_TICKER_APP_SLEEP_SCHEDULE_H
#define TOKEN_TICKER_APP_SLEEP_SCHEDULE_H

#include <stdbool.h>
#include <stdint.h>

#include "app_config.h"
#include "sensor_service.h"

bool app_sleep_schedule_is_enabled(const app_config_t *config);
bool app_sleep_schedule_is_active_now(const app_config_t *config,
                                      const rtc_time_t *rtc_time,
                                      bool *active);
bool app_sleep_schedule_seconds_until_transition(const app_config_t *config,
                                                 const rtc_time_t *rtc_time,
                                                 uint32_t *seconds_until_transition,
                                                 bool *next_is_sleep);
bool app_sleep_schedule_next_transition(const app_config_t *config,
                                        const rtc_time_t *rtc_time,
                                        rtc_time_t *next_transition_time,
                                        bool *next_is_sleep);

#endif