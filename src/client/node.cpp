#include "node.hpp"
#include "../ctg/ctg.hpp"
#include "../bitcoin_simgrid.hpp"

using json = nlohmann::json;

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

Node::Node(std::vector<std::string> args)
{
  init_from_args(args);
  simgrid::s4u::this_actor::on_exit(
    [](int, void*) {
      LOG("shut down. real simulation time: %ld seconds", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - START_TIME).count());
    }, nullptr
  );
}

void Node::init_from_args(std::vector<std::string> args)
{
  BaseNode::init_from_args(args);
  std::string mode = node_data["mode"].get<std::string>();
  using_trace = mode == "trace";
  if (using_trace) {
    trace = node_data["txs_trace"].get<std::vector<TraceItem>>();
  }
  difficulty = node_data["difficulty"].get<unsigned long long>();
  creates_txs = node_data["creates_txs"].get<bool>();
  xbt_assert(difficulty > 0, "Network difficulty must be greater than 0, got %llu", difficulty);
  do_set_next_activity_time();
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    get_peer_incoming_mailbox(peer_id)->set_receiver(simgrid::s4u::Actor::self());
  }
}

std::string Node::get_node_data_filename(int id) {
  return deployment_directory + simgrid::s4u::this_actor::get_name() + std::string("_data-") + std::to_string(my_id);
}

void Node::do_set_next_activity_time()
{
  if (creates_txs) {
      if (using_trace && (current_trace_index < trace.size())) {
        next_activity_item = trace[current_trace_index++];
        next_activity_time = next_activity_item.received;
        LOG("expect to create a tx %f", next_activity_time);
      } else {
        next_activity_item = ctg->get_next_activity_item(this);
        next_activity_time = next_activity_item.received;
      }
  } else {
    next_activity_time = SIMULATION_DURATION;
  }
}

double Node::get_next_activity_time()
{
  return next_activity_time;
}

void Node::generate_activity()
{
  if (next_activity_time > simgrid::s4u::Engine::get_clock()) {
    return;
  }
  std::map<long, Transaction> txs;
  Transaction tx = create_transaction(next_activity_item.size, next_activity_item.fee_per_byte, next_activity_item.confirmed);
  txs.insert(std::make_pair(tx.get_id(), tx));
  Transactions *my_unconfirmed_txs = new Transactions(txs);
  do_set_next_activity_time();
  handle_transactions(my_id, my_unconfirmed_txs);
  delete my_unconfirmed_txs;
}

Transaction Node::create_transaction(long size, long fee_per_byte, double confirmed)
{
  Transaction tx = Transaction(size, fee_per_byte, confirmed);
  LOG("creating tx %ld", tx.get_id());
  return tx;
}

bool Node::handle_messages()
{
  bool has_work_to_do = false;
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    has_work_to_do |= receive_messages_from_peer(peer_id);
    send_messages_to_peer(peer_id);
    cleanup(peer_id);
  }
  return has_work_to_do;
}

bool Node::receive_messages_from_peer(int peer_id)
{
  simgrid::s4u::MailboxPtr mbox = get_peer_incoming_mailbox(peer_id);
  if (!mbox->listen()) {
    return false;
  }
  void* data = mbox->get();
  received_messages++;
  bool has_work_to_do = mbox->ready();
  Message *payload = static_cast<Message*>(data);
  switch (payload->get_type()) {
    case MESSAGE_BLOCK:
      has_work_to_do |= handle_block(peer_id, static_cast<Block*>(data));
      break;
    case MESSAGE_TXS:
      has_work_to_do |= handle_transactions(peer_id, static_cast<Transactions*>(data));
      break;
    case MESSAGE_INV:
      handle_inv(peer_id, static_cast<Inv*>(data));
      break;
    case MESSAGE_GETDATA:
      handle_getdata(peer_id, static_cast<GetData*>(data));
      break;
    default:
      THROW_IMPOSSIBLE;
  }
  delete payload;
  return has_work_to_do;
}

/* We won't relay any messages in the next iteration unless:
* a) we have to notify that among our inventory we have a new block
* b) we have to notify that among our inventory we have new unconfirmed tx
* c) we need to inform our peer that we know of a new block or tx
* d) we need to request to our peer for a block or tx the peer know about
*/
void Node::send_messages_to_peer(int peer_id)
{
  send_blocks(peer_id);
  send_transactions(peer_id);
  inv(peer_id);
  getdata(peer_id);
}

void Node::send_blocks(int peer_id)
{
  // We will let the peer know about new blocks (but we won't send the blocks that we know the peer already knows)
  // The blocks I need to send other peers are those that a peer has requested to me, that I know about and that exist in the shared blocks variable
  std::map<long, Block> blocks_to_send = IntersectMaps(known_blocks, known_blocks_ids, objects_to_send_to_peer[peer_id]);
  blocks_known_by_peer[peer_id] = JoinMaps(blocks_known_by_peer[peer_id], blocks_to_send);
  typename std::map<long, Block>::const_iterator it_block = blocks_to_send.begin();
  while (it_block != blocks_to_send.end()) {
    sent_messages++;
    LOG("sending block %ld to %d", it_block->first, peer_id);
    simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
    Message *message = new Block(it_block->second);
    mbox->put_init(message, message->get_size())->detach();
    ++it_block;
  }
}

void Node::send_transactions(int peer_id)
{
  // We will let the peer know about recent unconfirmed txs (but we won't send the txs that we know the peer already knows)
  std::map<long, Transaction> txs_to_send = IntersectMaps(mempool, objects_to_send_to_peer[peer_id]);
  txs_known_by_peer[peer_id] = JoinMaps(txs_known_by_peer[peer_id], txs_to_send);
  if (txs_to_send.size() > 0) {
    sent_messages++;
    for (auto const& idAndTransaction : txs_to_send) {
      LOG("sending %ld tx to %d", idAndTransaction.first, peer_id);
    }
    Message *message = new Transactions(txs_to_send);
    simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
    mbox->put_init(message, message->get_size())->detach();
  }
}

// Here we're sending messages with the new inventory we know about
void Node::inv(int peer_id)
{
  std::map<long, e_inv_type> objects;
  std::set<long> blocks_ids_to_include = DiffSets(blocks_ids_to_broadcast[peer_id], blocks_known_by_peer[peer_id], objects_to_send_to_peer[peer_id]);
  std::set<long> txs_ids_to_include = DiffSets(txs_ids_to_broadcast[peer_id], txs_known_by_peer[peer_id], objects_to_send_to_peer[peer_id]);
  for (std::set<long>::iterator it_block_id = blocks_ids_to_include.begin(); it_block_id != blocks_ids_to_include.end(); it_block_id++) {
    objects.insert(std::make_pair(*it_block_id, INV_BLOCK));
  }
  for (std::set<long>::iterator it_tx_id = txs_ids_to_include.begin(); it_tx_id != txs_ids_to_include.end(); it_tx_id++) {
    objects.insert(std::make_pair(*it_tx_id, INV_TX));
  }
  if (objects.size() > 0) {
    sent_messages++;
    for (auto const& id : objects) {
      DEBUG("informing %d of %ld", peer_id, id.first);
    }
    Message *message = new Inv(objects);
    simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
    mbox->put_init(message, message->get_size())->detach();
  }
}

// Here we're sending messages with the inventory we need from other peers
void Node::getdata(int peer_id)
{
  // In objects_to_request we may have removed some of the objects we initially needed (because we later got them in a block or tx)
  // so we first intersect the object ids we need with the ones we are going to request from our peer
  std::set<long> filtered_objects = IntersectSets(objects_to_request_from_peer[peer_id], objects_to_request);
  if (filtered_objects.size() > 0) {
    for (auto const& id : filtered_objects) {
      DEBUG("requesting %ld from %d", id, peer_id);
    }
    sent_messages++;
    Message *message = new GetData(filtered_objects);
    simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
    mbox->put_init(message, message->get_size())->detach();
  }
}

// After a round of receiving and sending messages we need to clean-up some structures that we don't need anymore
void Node::cleanup(int peer_id)
{
  blocks_ids_to_broadcast[peer_id].clear();
  blocks_known_by_peer[peer_id].clear();
  txs_ids_to_broadcast[peer_id].clear();
  txs_known_by_peer[peer_id].clear();
  objects_to_send_to_peer[peer_id].clear();
  objects_to_request_from_peer[peer_id].clear();
}

bool Node::handle_block(int relayed_by_peer_id, Block *message, bool force_broadcast)
{
  bool new_work_to_do = false;
  Block block = Block(*message);
  // Remove the received block from the objects to request I have pending
  Erase(objects_to_request, block.get_id());
  if ((known_blocks_ids.find(block.get_id()) == known_blocks_ids.end()) || force_broadcast) {
    handle_new_block(relayed_by_peer_id, block);
    // I didn't know about this block, I need to check if it represents a new top for the blockchain
    if (blockchain_tip_updated(relayed_by_peer_id, block) || force_broadcast) {
      if (message->get_miner_id() == my_id) {
        LOG("broadcasting %ld with height %d and parent %ld", message->get_id(), message->get_height(), message->get_parent_id());
      }
      LOG(
        "received a new block %ld from %d with %ld txs",
        block.get_id(),
        relayed_by_peer_id,
        block.get_transactions_map().size()
      );
      handle_blockchain_tip_updated(relayed_by_peer_id, block);
      // Since we found a new block we'll have to broadcast its hash to our peers
      new_work_to_do = true;
    } else {
      LOG(
        "received a block %ld from %d with %ld txs which doesn't represent a new best chain",
        block.get_id(),
        relayed_by_peer_id,
        block.get_transactions_map().size()
      );
    }
  } else {
    // When a block arrives we only need to do something only if we didn't
    // know about it before.
    LOG(
      "received a known block %ld from %d with %ld transactions",
      block.get_id(),
      relayed_by_peer_id,
      block.get_transactions_map().size()
    );
  }
  handle_orphan_blocks(block);
  return new_work_to_do;
}

void Node::handle_new_block(int relayed_by_peer_id, const Block & block)
{
  // Fill the shared map nodes_knowing_block that we use for debugging purposes
  if (nodes_knowing_block.find(block.get_id()) == nodes_knowing_block.end()) {
    nodes_knowing_block[block.get_id()] = 1;
  } else {
    nodes_knowing_block[block.get_id()]++;
  }
  // We need to advertise our peers about the new block we received
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    blocks_ids_to_broadcast[*it_id].insert(block.get_id());
  }
  bool received_by_all = nodes_knowing_block[block.get_id()] == NODES_COUNT;
  if (received_by_all) {
    DEBUG("BLOCK_RECEIVED_BY_ALL %ld", block.get_id());
  }
}

void Node::handle_blockchain_tip_updated(int relayed_by_peer_id, const Block & block)
{
  // Set the new current network difficulty
  difficulty = block.get_network_difficulty();
  bool confirmed_by_all = nodes_knowing_block[block.get_id()] == NODES_COUNT;
  for (auto const& idAndTransaction : block.get_transactions_map()) {
    LOG("confirmed tx %ld in block %ld %s", idAndTransaction.first, block.get_id(), confirmed_by_all ? "FOR_ALL_NODES" : "");
  }
  // Remove from txs_ids_to_broadcast the ones that got confirmed in this block
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    txs_ids_to_broadcast[*it_id] = DiffSets(txs_ids_to_broadcast[*it_id], block.get_transactions_map());
  }
  // Now that we know of txs that got confirmed we need to evict them from our mempool
  mempool = DiffMaps(mempool, block.get_transactions_map());
  // Clean from the objects to requests any possible tx that we found about when we received the new block
  objects_to_request = DiffSets(objects_to_request, block.get_transactions_map());
  if (relayed_by_peer_id == my_id) {
    // I generated this block, so I'm a miner and I naturally want everyone to know
    // about this block as soon as possible
    for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
      int peer_id = *it_id;
      DEBUG("letting peer %d know about block %ld", peer_id, block.get_id());
      objects_to_send_to_peer[peer_id].insert(block.get_id());
    }
  } else {
    // This is a block I didn't generate, so I have to add it to the list of blocks known
    // by the peer who created it and I need to simulate the validation time
    blocks_known_by_peer[relayed_by_peer_id].insert(block.get_id());
    // Simulate the time we have to wait to validate this block
    double start = simgrid::s4u::Engine::get_clock();
    simgrid::s4u::this_actor::execute(validator_timer.get_flops_to_process_block(block));
    DEBUG("It took %f seconds to validate a block", simgrid::s4u::Engine::get_clock() - start);
  }
  // Remove from the unconfirmed transactions known by our peers those confirmed in the block we just received
  for(std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    txs_known_by_peer[peer_id] = DiffMaps(txs_known_by_peer[peer_id], block.get_transactions_map());
  }
  update_network_difficulty_if_needed(block);
}

void Node::update_network_difficulty_if_needed(const Block & block)
{
    if ((block.get_height() % INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS) == 0) {
      double expected_time = INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS * INTERVAL_BETWEEN_BLOCKS_IN_SECONDS;
      // We substract 1 to simulate the off-by-one bug error in the reference client implementation
      long known_block_id = known_blocks_ids_by_height.find(blockchain_height - (INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS - 1))->second;
      double base_time = known_blocks.find(known_block_id)->second.get_time();
      double actual_time = block.get_time() - base_time;
      unsigned long long new_difficulty = block.get_network_difficulty() * expected_time / actual_time;
      LOG(
        "blockchain_height %d previous difficulty %llu, new difficulty %llu block date %lf expected time %lf actual time %lf change %Lf",
        blockchain_height,
        difficulty,
        new_difficulty,
        block.get_time(),
        expected_time,
        actual_time,
        ((long double) new_difficulty) / ((long double) difficulty)
      );
      difficulty = new_difficulty;
    }
}

void Node::handle_orphan_blocks(Block block)
{
  // Check if this block is the parent of current orphan blocks
  if (orphan_blocks.find(block.get_id()) != orphan_blocks.end()) {
    std::vector<Block> orphans = orphan_blocks[block.get_id()];
    orphan_blocks[block.get_id()].clear();
    for(std::vector<Block>::iterator it_orphan = orphans.begin(); it_orphan != orphans.end(); it_orphan++) {
      DEBUG(
        "found parent %ld for %ld",
        block.get_id(),
        it_orphan->get_id()
      );
      Block orphan = *it_orphan;
      handle_block(my_id, &orphan);
    }
  }
}

bool Node::blockchain_tip_updated(int relayed_by_peer_id, Block block)
{
  if (known_blocks_ids.find(block.get_parent_id()) == known_blocks_ids.end()) {
    orphan_blocks[block.get_parent_id()].push_back(block);
    DEBUG("received orphan block %ld", block.get_id());
    // We have a block without its parent => request said block to the same peer, because if he
    // sent us this block is because he may have a chain with more proof of work than our current one
    request_block(relayed_by_peer_id, block.get_parent_id());
    return false;
  }
  known_blocks_ids.insert(block.get_id());
  known_blocks.insert(std::make_pair(block.get_id(), block));
  // Remove the received block from any possible object to request
  Erase(objects_to_request, block.get_id());
  // Check if we found a new best chain. We will accept the new block if its accumulated difficulty is
  // greather than the current one, ie: it represents a new best chain
  DEBUG(
    "difficulty comparison %llu vs %llu. first with id %ld:%d second with id %ld:%d",
    block.get_accumulated_difficulty(),
    known_blocks[blockchain_tip].get_accumulated_difficulty(),
    block.get_id(),
    block.get_height(),
    known_blocks[blockchain_tip].get_id(),
    known_blocks[blockchain_tip].get_height()
  );
  if (block.get_accumulated_difficulty() > known_blocks[blockchain_tip].get_accumulated_difficulty()) {
    if (block.get_parent_id() != blockchain_tip) {
      reorg_txs(block.get_id(), blockchain_tip);
    }
    blockchain_tip = block.get_id();
    blockchain_height = block.get_height();
    known_blocks_ids_by_height.insert(std::make_pair(block.get_height(), block.get_id()));
    return true;
  } else {
    return false;
  }
}

// When a block reorganization occurs I need to "forget" about known transactions that
// had got confirmed in the previous best chain. Then I need to learn/mark as confirmed
// the transactions that I only appeared in the new best chain.
void Node::reorg_txs(long new_tip_id, long old_tip_id)
{
  long common_parent_id = find_common_parent_id(new_tip_id, old_tip_id);
  long current_block_id = old_tip_id;
  int fork_length = 0;
  std::set<long> known_txs_to_discard;
  while (current_block_id != common_parent_id) {
    ++fork_length;
    Block block = known_blocks.find(current_block_id)->second;
    known_txs_to_discard = JoinSets(known_txs_to_discard, block.get_transactions_map());
    current_block_id = block.get_parent_id();
  }
  known_txs_ids = DiffSets(known_txs_ids, known_txs_to_discard);
  current_block_id = new_tip_id;
  std::set<long> known_txs_to_add;
  while (current_block_id != common_parent_id) {
    Block block = known_blocks.find(current_block_id)->second;
    known_txs_to_add = JoinSets(known_txs_to_add, block.get_transactions_map());
    current_block_id = block.get_parent_id();
  }
  known_txs_ids = JoinSets(known_txs_ids, known_txs_to_add);
  if (common_parent_id != old_tip_id) {
    LOG(
      "reorganizing blocks. new tip: %ld, old tip: %ld, common: %ld, fork length: %d, known txs discarded: %ld, known txs: added %ld",
      new_tip_id,
      old_tip_id,
      common_parent_id,
      fork_length,
      DiffSets(known_txs_to_discard, known_txs_to_add).size(),
      DiffSets(known_txs_to_add, known_txs_to_discard).size()
    );
  } else {
    LOG(
      "reorganizing blocks. new tip: %ld, known txs discarded: %ld, known txs: added %ld",
      common_parent_id,
      DiffSets(known_txs_to_discard, known_txs_to_add).size(),
      DiffSets(known_txs_to_add, known_txs_to_discard).size()
    );
  }
}

long Node::find_common_parent_id(long new_parent_tip_id, long old_parent_tip_id)
{
  std::set<long> parents_for_new_id = {new_parent_tip_id};
  std::set<long> parents_for_old_id = {old_parent_tip_id};
  while (IntersectSets(parents_for_new_id, parents_for_old_id).size() == 0) {
    new_parent_tip_id = known_blocks.find(new_parent_tip_id)->second.get_parent_id();
    old_parent_tip_id = known_blocks.find(old_parent_tip_id)->second.get_parent_id();
    parents_for_new_id.insert(new_parent_tip_id);
    parents_for_old_id.insert(old_parent_tip_id);
  }
  return *(IntersectSets(parents_for_new_id, parents_for_old_id).begin());
}

bool Node::handle_transactions(int relayed_by_peer_id, Transactions *message)
{
  std::map<long, Transaction> txs_we_didnt_know = DiffMaps(message->get_transactions_map(), known_txs_ids);
  // Remove the received txs from any possible object to request
  objects_to_request = DiffSets(objects_to_request, txs_we_didnt_know);
  for (auto const& idAndTransaction : txs_we_didnt_know) {
    LOG("received tx %ld from %d", idAndTransaction.first, relayed_by_peer_id);
  }
  known_txs_ids = JoinMaps(known_txs_ids, message->get_transactions_map());
  // The transactions to broadcast will now also include the ones I didn't know of before
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    txs_ids_to_broadcast[*it_id] = JoinSets(txs_ids_to_broadcast[*it_id], txs_we_didnt_know);
  }
  // The transactions I'm aware of now include the ones I just received
  mempool = JoinMaps(mempool, message->get_transactions_map());
  if (relayed_by_peer_id != my_id) {
    // Now I need to update the txs that I know my peer knows about
    txs_known_by_peer[relayed_by_peer_id] = JoinMaps(txs_known_by_peer[relayed_by_peer_id], message->get_transactions_map());
  }
  bool has_work_to_do = txs_we_didnt_know.size() > 0;
  if (has_work_to_do) {
    // Simulate validation time for txs we didn't know about
    double start = simgrid::s4u::Engine::get_clock();
    simgrid::s4u::this_actor::execute(validator_timer.get_flops_to_process_transactions(txs_we_didnt_know));
    DEBUG("It took %f seconds to validate txs", simgrid::s4u::Engine::get_clock() - start);
  }
  return has_work_to_do;
}

// Other peer is informing us about some inventory he knows about. If we don't know about some object we're
// going to request it from said peer
void Node::handle_inv(int relayed_by_peer_id, Inv *message)
{
  std::map<long, e_inv_type> objects_received = message->get_objects();
  for(std::map<long, e_inv_type>::iterator it_object = objects_received.begin(); it_object != objects_received.end(); it_object++) {
    // Add the objects we don't know about yet only if we are not already going to ask for it to another peer
    if (objects_to_request.find(it_object->first) == objects_to_request.end()) {
      switch (it_object->second) {
        case INV_BLOCK:
          blocks_known_by_peer[relayed_by_peer_id].insert(it_object->first);
          if (known_blocks_ids.find(it_object->first) == known_blocks_ids.end()) {
            request_block(relayed_by_peer_id, it_object->first);
          }
          break;
        case INV_TX:
          txs_known_by_peer[relayed_by_peer_id].insert(it_object->first);
          if (known_txs_ids.find(it_object->first) == known_txs_ids.end()) {
            DEBUG(
              "need to request tx %ld from %d",
              it_object->first,
              relayed_by_peer_id
            );
            // I don't know about this tx => I will ask the peer to send it to me
            objects_to_request.insert(it_object->first);
            objects_to_request_from_peer[relayed_by_peer_id].insert(it_object->first);
          }
          break;
        default:
          THROW_IMPOSSIBLE;
      }
    }
  }
}

void Node::request_block(int relayed_by_peer_id, long block_id) {
  // I don't know about this block => I will ask the peer to send it to me
  DEBUG(
    "need to request block %ld from %d",
    block_id,
    relayed_by_peer_id
  );
  objects_to_request.insert(block_id);
  objects_to_request_from_peer[relayed_by_peer_id].insert(block_id);
}

// Other peer is asking that we send him inventory that we know about
void Node::handle_getdata(int relayed_by_peer_id, GetData *message)
{
  for (auto const& id : message->get_objects()) {
    DEBUG("node %d requested %ld", relayed_by_peer_id, id);
  }
  objects_to_send_to_peer[relayed_by_peer_id] = JoinSets(objects_to_send_to_peer[relayed_by_peer_id], message->get_objects());
}

long Node::compute_mempool_size()
{
  long result = 0;
  typename std::map<long, Transaction>::const_iterator it = mempool.begin();
  while (it != mempool.end()) {
    result += it->second.get_size();
    ++it;
  }
  return result;
}

simgrid::s4u::MailboxPtr Node::get_peer_incoming_mailbox(int peer_id)
{
  std::string mboxName = std::string("from:") + std::to_string(peer_id) + "-to:" + std::to_string(my_id);
  return simgrid::s4u::Mailbox::by_name(mboxName);
}

simgrid::s4u::MailboxPtr Node::get_peer_outgoing_mailbox(int peer_id)
{
  std::string mboxName = std::string("from:") + std::to_string(my_id) + "-to:" + std::to_string(peer_id);
  return simgrid::s4u::Mailbox::by_name(mboxName);
}
