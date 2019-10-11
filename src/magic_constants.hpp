#ifndef MAGIC_CONSTANTS
#define MAGIC_CONSTANTS
#include <random>

// This it the total time in seconds we are going to allow the simulation to run. Initialized from bitcoin_simgrid.cpp
extern unsigned int SIMULATION_DURATION;

// This it the sum of nodes (normal and miners) that are part of the current simulation. Initialized from bitcoin_simgrid.cpp
extern unsigned int NODES_COUNT;

// We try to imitate ThreadMessageHandler from the reference client  https://github.com/bitcoin/bitcoin/blob/a7324bd/src/net.cpp
// In ThreadMessageHandler we first process all pending messages and then send all needed messages to our peers once every 100 milliseconds
// Initialized from bitcoin_simgrid.cpp
extern double SLEEP_DURATION;

// Used as parameter for the Poisson distribution that will generate the blocks. Initialized from bitcoin_simgrid.cpp
extern unsigned int INTERVAL_BETWEEN_BLOCKS_IN_SECONDS;

// Every these many blocks, the network will calculate the new difficulty
static const unsigned int INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS = 2016;

// How many bytes we expect any given transaction size to be (on average)
static const unsigned int AVERAGE_BYTES_PER_TX = 1500;

// How much fee we expect any given transaction to be (on average). Right now only useful when the txs in the mempool
// don't fit in a block and the miner needs to chose the subset of txs with the highest fee
static const unsigned int AVERAGE_FEE_PER_BYTE = 200;

// Every message should have at least these many bytes. Useful for example for INV and GETDATA messages
static const unsigned int BASE_MSG_SIZE = 80;

// Limit the maximum block size to 1MB, following the limit from the reference client
static const unsigned int MAX_BLOCK_SIZE = 1048576;

// JSON encoded number are more limited than C++ ones and can't represent legitimate high values.
// So the tool accepts lower JSON encoded hashrate values that can then be up-scaled using this constant
extern unsigned int HASHRATE_SCALE;

// If true, then we will avoid the loop events of each node when we know there are no more messages to receive/send
// until the next global activity in the network
extern bool SKIP_TIME_WHEN_POSSIBLE;

// If true, more information about transactions and blocks will be included in the produced log
extern bool ENABLE_DEBUG;

// Allow to set random generator seed in order to reproduce simulations in a deterministic way
extern unsigned int SEED;

// Random number generator, will be initialized using SEED value
extern std::default_random_engine re;

#endif /* MAGIC_CONSTANTS */
