#ifndef MINER_HPP
#define MINER_HPP

#include "node.hpp"
#include "../trace/trace_item_miner.hpp"

/*
* This represent a miner (which has all the functionality of a node) that knows how to
* - create blocks
* - set the time for the next block generation
*/
class Miner : public Node {
public:
  explicit Miner(std::vector<std::string> args);
  double get_next_activity_time();

protected:
  void init_from_args(std::vector<std::string> args);
  void generate_activity();
  bool handle_block(int relayed_by_peer_id, Block *message, bool force_broadcast = false);

private:
  // Whether we should create blocks following a model based on our hashreate the network difficulty
  // or based on a real blockchain trace
  bool using_trace;
  // If I'm generating the blocks following a real blockchain trace, this is where I store the index of
  // the next acitivity item that need to be user to create the new block
  int current_trace_index = 0;
  // If I'm generating the blocks following a real blockchain trace, this is where I store the trace information
  std::vector<TraceItemMiner> trace;
  // If I'm generating the block following a model, the more hashpower I have the more frequent this miner
  // will create new blocks
  unsigned long long hashrate;
  // The next block should be create at next_activity_time
  double next_activity_time;
  // Whether this miner will use selfish mining strategy
  bool using_selfish_mining;
  // Blocks we've mined but are withholding from the public by following a selfish mining strategy
  std::map<int, Block> pending_blocks;
  // This represents the highest block from the alternative mainchain being built by miners other than me
  int best_competing_height = 0;

  void do_set_next_activity_time();
  double get_event_probability();
  void add_mempool_transactions(std::vector<Transaction> &txs_to_include, double confirmation_time);
  long get_transactions_size(std::vector<Transaction> txs);
  void restrict_transactions_to_available_size(std::vector<Transaction> &txs, long available_size);
  void clean_pending_blocks();
  bool announce_pending_blocks(int up_to_height);
};

#endif /* MINER_HPP */
