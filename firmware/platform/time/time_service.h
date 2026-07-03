#ifndef TOKEN_TICKER_TIME_SERVICE_H
#define TOKEN_TICKER_TIME_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

#define TIME_SERVICE_TZ_LEN 48

typedef struct
{
    bool ntp_enabled;
    uint16_t ntp_sync_hours;
    int64_t last_ntp_sync_epoch;
    int64_t last_ntp_sync_monotonic_epoch;
    bool time_valid;
    char timezone[TIME_SERVICE_TZ_LEN];
} time_service_state_t;

void time_service_init(bool ntp_enabled, uint16_t ntp_sync_hours, const char *timezone);
void time_service_mark_time_valid(int64_t epoch_seconds, int64_t monotonic_seconds);
bool time_service_should_sync_ntp(int64_t now_epoch_seconds);
bool time_service_sync_ntp(uint32_t timeout_ms, int64_t monotonic_seconds, int64_t *epoch_seconds);
const time_service_state_t *time_service_get_state(void);

#endif