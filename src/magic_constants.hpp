#ifndef MAGIC_CONSTANTS
#define MAGIC_CONSTANTS

// Some short amount of sleep when there's nothing left to do
static const int SLEEP_DURATION = 1;

// These constants were retrieved from the reference client  https://github.com/bitcoin/bitcoin/blob/92fabcd/src/validation.h
static const unsigned int INVENTORY_BROADCAST_INTERVAL = 5;
static const unsigned int INVENTORY_BROADCAST_MAX = 7 * INVENTORY_BROADCAST_INTERVAL;

#endif /* MAGIC_CONSTANTS */
