#include "shared_data.hpp"

// This is the shared (among all nodes and miner) map of blocks we know about
// It's indexed by the block id
std::map<long, Block> known_blocks = {{0, Block()}};

// In this map we'll store the number of nodes knowing about each block.
// This is specially usefull for debugging purpuses to log when a block has
// reached the global consensus of the network.
std::map<long, int> nodes_knowing_block = {};


// <PERFORMANCE_IMPROVEMENTS>
int perf_improv_stage = PERF_STAGE_INIT;

double next_time_for_global_activity = 0;

int sent_messages = 0;

int received_messages = 0;

int next_times_set = 0;

std::set<long> long_sleep_completed_for_node_id = {};
// </PERFORMANCE_IMPROVEMENTS>
