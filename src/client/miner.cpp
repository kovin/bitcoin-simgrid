#include "miner.hpp"
#include "../bitcoin_simgrid.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

Miner::Miner(std::vector<std::string> args): Node::Node()
{
  init_from_args(args);
  simgrid::s4u::this_actor::on_exit(
    [](int, void*) {
      LOG("shut down. real simulation time: %ld seconds", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - START_TIME).count());
    }, nullptr
  );
}

void Miner::init_from_args(std::vector<std::string> args)
{
  Node::init_from_args(args);
  std::string mode = node_data["mode"].get<std::string>();
  using_trace = mode == "trace";
  using_selfish_mining = mode == "model_using_selfish_mining";
  if (using_trace) {
    trace = node_data["trace"].get<std::vector<TraceItemMiner>>();
  } else {
    hashrate = node_data["hashrate"].get<unsigned long long>();
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
      LOG("expect to create a block with %d txs at %f", trace[current_trace_index - 1].n_tx, next_activity_time);
    } else {
      // There are no more blocks to simulate => return SIMULATION_DURATION to avoid this miner from generating more blocks
      next_activity_time = SIMULATION_DURATION;
    }
  } else {
    int timespan = INTERVAL_BETWEEN_BLOCKS_IN_SECONDS;// 10 minutes by default if not using the --target-time option
    double event_probability = get_event_probability();
    next_activity_time = calc_next_activity_time(next_activity_time, event_probability, timespan, HASHRATE_SCALE);
    LOG("next activity will be %f", next_activity_time);
  }
}

double Miner::get_event_probability()
{
  // Ideal global hashrate to match current network difficulty ( https://bitcoin.stackexchange.com/a/5557 )
  double ideal_global_hashrate = difficulty / 600;
  double adapted_miner_hashrate = hashrate >> 32;
  double event_probability = adapted_miner_hashrate / ideal_global_hashrate;
  return event_probability;
}

bool Miner::handle_block(int relayed_by_peer_id, Block *message, bool force_broadcast)
{
  if (using_selfish_mining) {
    bool returnValue = false;
    if (message->get_miner_id() == my_id) {
      Node::blockchain_tip_updated(relayed_by_peer_id, Block(*message));
      if (message->get_height() > best_competing_height) {
        // The block I mined is the best chain so far AND is ahead by at least 1 from others
        // I have a cushion to keep withholding this block and only reveal it when needed to
        // maintain the status quo
        DEBUG("[selfish mining] our height %d vs %d from others", message->get_height(), best_competing_height);
        pending_blocks.insert(std::make_pair(message->get_id(), Block(*message)));
        Node::blockchain_tip_updated(relayed_by_peer_id, Block(*message));
        return false;
      }
    } else {
      best_competing_height = std::max(best_competing_height, message->get_height());
      if (Node::blockchain_tip_updated(relayed_by_peer_id, Block(*message))) {
        // We received a block from other miner which represents a better chain than ours
        // Discard our private chain
        pending_blocks.clear();
      } else {
        // We received a messsage from others which doesn't represent a better chain than ours.
        // From our pending blocks publish message->get_height() + 1 to keep the status quo
        DEBUG("[selfish mining] keeping status quo at height %d", message->get_height());
        return announce_pending_blocks(message->get_height() + 1);
      }
    }
  }
  // I'm not using selfish mining or I have to accept an alternative chain better than mine
  return Node::handle_block(relayed_by_peer_id, message, message->get_miner_id() == my_id);
}

bool Miner::announce_pending_blocks(int up_to_height)
{
  std::map<int, Block> new_pending_blocks;
  typename std::map<int, Block>::const_iterator it = pending_blocks.begin();
  while (it != pending_blocks.end()) {
    Block block = it->second;
    if (block.get_height() <= up_to_height) {
      DEBUG("announcing %ld of height %d <= %d", block.get_id(), block.get_height(), up_to_height);
      Node::handle_block(my_id, new Block(block), true);
    } else {
      DEBUG("not announcing %ld of height %d > %d", block.get_id(), block.get_height(), up_to_height);
      new_pending_blocks.insert(std::make_pair(block.get_id(), block));
    }
    ++it;
  }
  bool some_work_done = pending_blocks.size() != new_pending_blocks.size();
  pending_blocks = new_pending_blocks;
  return some_work_done;
}

void Miner::generate_activity()
{
  Node::generate_activity();
  if (next_activity_time > simgrid::s4u::Engine::get_clock()) {
    return;
  }
  Block *block;
  std::vector<Transaction> txs_to_include;
  unsigned long long accumulated_difficulty = known_blocks[blockchain_tip].get_accumulated_difficulty() + difficulty;
  if (using_trace) {
    // I need to add to the block the coinbase tx and all the txs that only appeared
    // in the network when this block was broadcasted
    TraceItemMiner traceItem = trace[current_trace_index - 1];
    for (auto const& size_and_fee : traceItem.txs_broadcasted_in_block) {
      Transaction tx = create_transaction(size_and_fee.first, size_and_fee.second, next_activity_time);
      txs_to_include.push_back(tx);
    }
    add_mempool_transactions(txs_to_include, next_activity_time);
    block = new Block(blockchain_height + 1, simgrid::s4u::Engine::get_clock(), blockchain_tip, difficulty, accumulated_difficulty, txs_to_include, my_id);
    LOG("creating block %ld with %ld txs and we expected %d. height: %d, parent %ld", block->get_id(), txs_to_include.size(), traceItem.n_tx, block->get_height(), block->get_parent_id());
  } else {
    // I need to include the coinbase tx
    long size = lrand(AVERAGE_BYTES_PER_TX * 2);// On average txs size will be AVERAGE_BYTES_PER_TX bytes
    long fee_per_byte = lrand(AVERAGE_FEE_PER_BYTE * 2);// On average txs fee per byte will be AVERAGE_FEE_PER_BYTE bytes
    Transaction tx = create_transaction(size, fee_per_byte, next_activity_time);
    txs_to_include.push_back(tx);
    add_mempool_transactions(txs_to_include, next_activity_time);
    block = new Block(blockchain_height + 1, simgrid::s4u::Engine::get_clock(), blockchain_tip, difficulty, accumulated_difficulty, txs_to_include, my_id);
    LOG("creating block %ld with %ld txs. height: %d, parent %ld", block->get_id(), txs_to_include.size(), block->get_height(), block->get_parent_id());
  }
  mempool = JoinMaps(mempool, block->get_transactions_map());
  do_set_next_activity_time();
  handle_block(my_id, block);
  delete block;
}

void Miner::add_mempool_transactions(std::vector<Transaction> &txs_to_include, double confirmation_time)
{
  std::vector<Transaction> mempool_txs_to_add;
  for (auto const& idAndTransaction : mempool) {
    // Only include txs that were confirmed since this block was created . If we're reproducing a trace we know when they were
    // confirmed in the "real blockchain"). In that situation, this check would ideally mean including them in the very same
    // block where they were confirmed (if the tx had time to reach the miner since it was broadcasted by the node that created it)
    if (confirmation_time >= idAndTransaction.second.get_confirmed()) {
      mempool_txs_to_add.push_back(idAndTransaction.second);
    }
  }
  sort(mempool_txs_to_add.begin(), mempool_txs_to_add.end(), [](const Transaction & a, const Transaction & b) -> bool
  {
    return b.get_fee_per_byte() > a.get_fee_per_byte();
  });
  long available_size = MAX_BLOCK_SIZE - get_transactions_size(txs_to_include);
  if (get_transactions_size(mempool_txs_to_add) > available_size) {
    restrict_transactions_to_available_size(mempool_txs_to_add, available_size);
  }
  txs_to_include.insert(txs_to_include.end(), mempool_txs_to_add.begin(), mempool_txs_to_add.end());
}

long Miner::get_transactions_size(std::vector<Transaction> txs)
{
  long size = 0;
  for (auto const& tx : txs) {
    size += tx.get_size();
  }
  return size;
}

void Miner::restrict_transactions_to_available_size(std::vector<Transaction> &txs, long available_size)
{
  std::vector<Transaction> result;
  long size = 0;
  for (auto const& tx : txs) {
    if ((size + tx.get_size()) > available_size) {
      break;
    }
    size += tx.get_size();
    result.push_back(tx);
  }
  txs = result;
}
