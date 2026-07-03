#include "provider_snapshot.h"

#include <string.h>

void provider_snapshot_init(provider_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return;
    }

    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->provider_type = PROVIDER_TYPE_UNKNOWN;
    snapshot->sync_state = PROVIDER_SYNC_STATE_IDLE;
    snapshot->primary_window_index = 0;
}