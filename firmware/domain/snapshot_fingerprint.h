#ifndef TOKEN_TICKER_SNAPSHOT_FINGERPRINT_H
#define TOKEN_TICKER_SNAPSHOT_FINGERPRINT_H

#include <stdint.h>

#include "provider_snapshot.h"

uint32_t provider_snapshot_fingerprint(const provider_snapshot_t *snapshot);

#endif