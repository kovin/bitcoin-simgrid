#ifndef MAGIC_CONSTANTS
#define MAGIC_CONSTANTS

// This it the total time in seconds we are goind to allow the simulation to run
extern unsigned int SIMULATION_DURATION;

// We try to imitate ThreadMessageHandler from the reference client  https://github.com/bitcoin/bitcoin/blob/a7324bd/src/net.cpp
// In ThreadMessageHandler we first process all pending messages and then send all needed messages to our peers once every 100 milliseconds
static const double SLEEP_DURATION = .1;

// These constants were retrieved from the reference client  https://github.com/bitcoin/bitcoin/blob/92fabcd/src/validation.h
static const unsigned int INVENTORY_BROADCAST_INTERVAL = 5;
static const unsigned int INVENTORY_BROADCAST_MAX = 7 * INVENTORY_BROADCAST_INTERVAL;

static const unsigned int INTERVAL_BETWEEN_BLOCKS_IN_SECONDS = 600;

static const unsigned int INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS = 2016;

static const unsigned int AVERAGE_BYTES_PER_TX = 1500;

static const unsigned int BASE_MSG_SIZE = 100;// Every message will have at least this many bytes

#endif /* MAGIC_CONSTANTS */
