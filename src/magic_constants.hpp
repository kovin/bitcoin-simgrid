#ifndef MAGIC_CONSTANTS
#define MAGIC_CONSTANTS

// We try to imitate ThreadMessageHandler from the reference client  https://github.com/bitcoin/bitcoin/blob/a7324bd/src/net.cpp
// In ThreadMessageHandler we first process all pending messages and then send all needed messages to our peers once every 100 milliseconds
static const double SLEEP_DURATION = .1;

// These constants were retrieved from the reference client  https://github.com/bitcoin/bitcoin/blob/92fabcd/src/validation.h
static const unsigned int INVENTORY_BROADCAST_INTERVAL = 5;
static const unsigned int INVENTORY_BROADCAST_MAX = 7 * INVENTORY_BROADCAST_INTERVAL;

#endif /* MAGIC_CONSTANTS */
