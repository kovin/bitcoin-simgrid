#ifndef SHARED_DATA_HPP
#define SHARED_DATA_HPP

#include "../message.hpp"

#define PERF_STAGE_INIT 0
#define PERF_STAGE_MESSAGES_OK 1
#define PERF_STAGE_NEXT_ACTIVITY_OK 2


// Here we define the set of structures that will be shared among nodes and miners, given
// that there's not reason to waste memory duplicating the knwon objects.
// In each node/miner we just need to have the set of "locally" knows txs and blocks but
// the corresponding object will then be retrieved from this shared source

// Map of block-id => block that have been broadcasted
extern std::map<long, Block> known_blocks;

// Number of nodes knowing about individual broadcasted blocks
extern std::map<long, int> nodes_knowing_block;

extern int perf_improv_stage;

extern double next_time_for_global_activity;

extern int sent_messages;

extern int received_messages;

extern int next_times_set;

extern std::set<long> long_sleep_completed_for_node_id;

#endif /* SHARED_DATA_HPP */
