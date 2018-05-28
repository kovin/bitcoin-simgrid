#include "miner.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

Miner::Miner(std::vector<std::string> args): Node::Node()
{
  init_from_args(args);
  simgrid::s4u::this_actor::on_exit((int_f_pvoid_pvoid_t) on_exit, NULL);
}

void Miner::init_from_args(std::vector<std::string> args)
{
  Node::init_from_args(args);
  std::string mode = node_data["mode"].get<std::string>();
  using_trace = mode == "trace";
  if (using_trace) {
    trace = node_data["trace"].get<std::vector<TraceItemMiner>>();
  } else {
    hashrate = node_data["hashrate"].get<long long>();
    xbt_assert(hashrate >= 0, "Miner hashrate can't be negative, got %lld", hashrate);
  }
  do_set_next_activity_time();
}

double Miner::get_next_activity_time()
{
  return std::min(next_activity_time, Node::get_next_activity_time());
}

// We set next time accoriding to the probability of this miner finding a block in the next 10 minutes
void Miner::do_set_next_activity_time()
{
  if (using_trace) {
    if (current_trace_index < trace.size()) {
      next_activity_time = trace[current_trace_index++].received;
      std::cout << "espero generar " << trace[current_trace_index - 1].n_tx << " txs" << std::endl;
      std::cout << "espero incluir al menos " << trace[current_trace_index - 1].n_tx_only_in_block << " txs" << std::endl;
      std::cout << next_activity_time << std::endl;
    } else {
      // There are no more blocks to simulate => return SIMULATION_DURATION to avoid this miner from generating more blocks
      next_activity_time = SIMULATION_DURATION;
    }
  } else {
    int timespan = INTERVAL_BETWEEN_BLOCKS_IN_SECONDS; // 10 minutes
    double event_probability = get_event_probability(timespan);
    double wasted_time = simgrid::s4u::Engine::get_clock() - next_activity_time;
    next_activity_time = calc_next_activity_time(event_probability, timespan, 1) - wasted_time;
  }
}

double Miner::get_event_probability(int timespan)
{
  double shares = (hashrate / pow(2, 32)) * timespan ;
  return 1 - pow(1 - 1.0 / difficulty, shares);
}

void Miner::generate_activity()
{
  Node::generate_activity();
  if (next_activity_time > simgrid::s4u::Engine::get_clock()) {
    return;
  }
  std::cout << "next activity time" << next_activity_time << std::endl;
  do_set_next_activity_time();
  if (using_trace) {
    // I need to add to the block the coinbase tx and all the txs that only appeared
    // in the network when this block was broadcasted
    TraceItemMiner traceItem = trace[current_trace_index - 1];
    for (int i = 0; i < traceItem.n_tx_only_in_block; i++) {
      Transaction tx = create_transaction();
      mempool.insert(std::make_pair(tx.id, tx));
    }
    XBT_INFO("creating a block with %ld txs and we expected %d", mempool.size(), traceItem.n_tx);
  } else {
    // I need to include the coinbase tx
    Transaction tx = create_transaction();
    mempool.insert(std::make_pair(tx.id, tx));
    XBT_INFO("creating a block with %ld txs", mempool.size());
  }
  Block *block = new Block(my_id, blockchain_height + 1, simgrid::s4u::Engine::get_clock(), blockchain_tip, difficulty, mempool);
  handle_new_block(my_id, block);
  delete block;
}
