#include "node.hpp"
#include "ctg.hpp"
#include "bitcoin_simgrid.hpp"

using json = nlohmann::json;

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

Node::Node(std::vector<std::string> args)
{
  init_from_args(args);
  simgrid::s4u::this_actor::on_exit((int_f_pvoid_pvoid_t) on_exit, NULL);
}

void Node::init_from_args(std::vector<std::string> args)
{
  BaseNode::init_from_args(args);
  difficulty = node_data["difficulty"].get<long long>();
  xbt_assert(difficulty > 0, "Network difficulty must be greater than 0, got %lld", difficulty);
  do_set_next_activity_time();
}

std::string Node::get_node_data_filename(int id) {
  return deployment_directory + simgrid::s4u::this_actor::get_name() + std::string("_data-") + std::to_string(my_id);
}

void Node::do_set_next_activity_time()
{
  next_activity_time = ctg->get_next_activity_time(this);
}

double Node::get_next_activity_time()
{
  return next_activity_time;
}

/* We won't relay any messages in the next iteration unless:
* a) this node creates a tx
* b) we receive a block we didn't know about
* c) we receive unconfirmed txs that are not present in our mempool
*/
void Node::send_messages()
{
  send_blocks();
  send_transactions();
  inv();
  getdata();
}

void Node::send_blocks()
{
  // We will let each peer know about new blocks (but we won't send the blocks that we know the peer already knows)
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    std::map<long, Block> blocks_to_send = IntersectMaps(known_blocks_by_id, objects_to_send_to_peer[peer_id]);
    blocks_known_by_peer[peer_id] = JoinMaps(blocks_known_by_peer[peer_id], blocks_to_send);
    typename std::map<long, Block>::const_iterator it_block = blocks_to_send.begin();
    while (it_block != blocks_to_send.end()) {
      XBT_INFO("sending block %ld to %d", it_block->first, peer_id);
      simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
      mbox->put_async(new Block(it_block->second), BASE_MSG_SIZE + it_block->second.size);
      ++it_block;
    }
  }
  typename std::map<long, Block>::const_iterator it_block = blocks_to_broadcast.begin();
  while (it_block != blocks_to_broadcast.end()) {
    // Remove from txs_to_broadcast the ones that got confirmed in this block
    txs_to_broadcast = DiffMaps(txs_to_broadcast, it_block->second.transactions);
    ++it_block;
  }
}

void Node::send_transactions()
{
  // We will let each peer know about recent unconfirmed txs (but we won't send the txs that we know the peer already knows)
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    std::map<long, Transaction> txs_to_send = IntersectMaps(mempool, objects_to_send_to_peer[peer_id]);
    txs_known_by_peer[peer_id] = JoinMaps(txs_known_by_peer[peer_id], txs_to_send);
    if (txs_to_send.size() > 0) {
      XBT_INFO("sending %ld unconfirmed transactions to %d", txs_to_send.size(), peer_id);
      Message *message = new UnconfirmedTransactions(my_id, txs_to_send);
      simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
      mbox->put_async(message, BASE_MSG_SIZE + message->size);
    }
  }
}

// Here we're sending messages with the new inventory we know about
void Node::inv()
{
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    std::map<long, e_inv_type> objects;
    std::map<long, Block> blocks_to_include = DiffMaps(blocks_to_broadcast, blocks_known_by_peer[peer_id]);
    std::map<long, Transaction> txs_to_include = DiffMaps(txs_to_broadcast, txs_known_by_peer[peer_id]);
    for (std::map<long, Block>::iterator it_block = blocks_to_include.begin(); it_block != blocks_to_include.end(); it_block++) {
      objects.insert(std::make_pair(it_block->first, INV_BLOCK));
    }
    for (std::map<long, Transaction>::iterator it_tx = txs_to_include.begin(); it_tx != txs_to_include.end(); it_tx++) {
      objects.insert(std::make_pair(it_tx->first, INV_TX));
    }
    if (objects.size() > 0) {
        Message *message = new Inv(my_id, objects);
        simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
        mbox->put_async(message, BASE_MSG_SIZE);
    }
  }
}

// Here we're sending messages with the inventory we need from other peers
void Node::getdata()
{
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    // In objects_ids_to_request we may have removed some of the objects we initially needed (because we later got them in a block or tx)
    // so we first intersect the object ids we need with the ones we are going to request our peer
    std::map<long, e_inv_type> objects_to_request = IntersectMaps(objects_to_request_from_peer[peer_id], objects_ids_to_request);
    if (objects_to_request.size() > 0) {
        Message *message = new GetData(my_id, objects_to_request);
        simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
        mbox->put_async(message, BASE_MSG_SIZE);
    }
  }
}

// After a round of receiving and sending messages we need to clean-up some structures that we don't need anymore
void Node::cleanup()
{
  blocks_to_broadcast.clear();
  blocks_known_by_peer.clear();
  txs_to_broadcast.clear();
  txs_known_by_peer.clear();
  objects_to_send_to_peer.clear();
  objects_ids_to_request.clear();
  objects_to_request_from_peer.clear();
}

void Node::generate_activity()
{
  if (next_activity_time > simgrid::s4u::Engine::get_clock()) {
    return;
  }
  do_set_next_activity_time();
  std::map<long, Transaction> txs;
  Transaction tx = create_transaction();
  txs.insert(std::make_pair(tx.id, tx));
  UnconfirmedTransactions *my_unconfirmed_txs = new UnconfirmedTransactions(my_id, txs);
  handle_transactions(my_id, my_unconfirmed_txs);
  delete my_unconfirmed_txs;
}

Transaction Node::create_transaction()
{
  long numberOfBytes = rand() & AVERAGE_BYTES_PER_TX;
  Transaction tx = Transaction(my_id, numberOfBytes);
  XBT_INFO("creating tx %ld", tx.id);
  return tx;
}

void Node::process_messages()
{
  std::vector<simgrid::s4u::CommPtr> pending_recvs;
  simgrid::s4u::CommPtr comm;
  std::vector<int> corresponding_peer_id;
  int messages_to_receive = 0;
  int received_messages = 0;
  void* data;
  for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    simgrid::s4u::MailboxPtr mbox = get_peer_incoming_mailbox(peer_id);
    if (!mbox->empty()) {
      messages_to_receive++;
      comm = mbox->get_async(&data);
      pending_recvs.push_back(comm);
      corresponding_peer_id.push_back(peer_id);
    }
  }
  while (received_messages < messages_to_receive) {
    int idx = simgrid::s4u::Comm::wait_any(&pending_recvs);
    if (idx != -1) {
      received_messages++;
      comm = pending_recvs.at(idx);
      int peer_id = corresponding_peer_id.at(idx);
      pending_recvs.erase(pending_recvs.begin() + idx);
      corresponding_peer_id.erase(corresponding_peer_id.begin() + idx);
      Message *payload = static_cast<Message*>(data);
      switch (payload->get_type()) {
        case MESSAGE_BLOCK:
          handle_new_block(peer_id, static_cast<Block*>(data));
          break;
        case UNCONFIRMED_TRANSACTIONS:
          handle_transactions(peer_id, static_cast<UnconfirmedTransactions*>(data));
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
    }
  }
}

void Node::handle_new_block(int relayed_by_peer_id, Block *message)
{
  Block block = Block(*message);
  if (known_blocks_by_id.find(block.id) == known_blocks_by_id.end()) {
    // I didn't know about this block, I need to check if it represents a new top for the blockchain
    if (blockchain_tip_updated(block)) {
      long previous_mempool_size = mempool.size();
      // Now that we know of txs that got confirmed we need to evict them from our mempool
      mempool = DiffMaps(mempool, block.transactions);
      // Clean from the objects to requests any possible tx that we found about when we received the new block
      objects_ids_to_request = DiffSets(objects_ids_to_request, JustKeys(block.transactions));
      long known_txs_that_got_confirmed = previous_mempool_size - mempool.size();
      XBT_INFO(
        "received block %ld from %d with %ld transactions (%ld of them new). mempool size: %ld",
        block.id,
        relayed_by_peer_id,
        block.transactions.size(),
        block.transactions.size() - known_txs_that_got_confirmed,
        mempool.size()
      );
      blocks_to_broadcast.insert(std::make_pair(block.id, block));
      if (relayed_by_peer_id == my_id) {
        // I generated this block, so I'm a miner and I naturally want everyone to know
        // about this block as soon as possible
        for (std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
          int peer_id = *it_id;
          objects_to_send_to_peer[peer_id].insert(std::make_pair(block.id, INV_BLOCK));
        }
      } else {
        // This is a block I didn't generate, so I have to add it to the list of blocks known
        // by the peer who created it and I need to simulate the validation time
        blocks_known_by_peer[relayed_by_peer_id].insert(block.id);
        simgrid::s4u::this_actor::execute(get_time_to_process_block(block));
      }
      // Remove from the unconfirmed transactions known by our peers those confirmed in the block we just received
      for(std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
        int peer_id = *it_id;
        txs_known_by_peer[peer_id] = DiffMaps(txs_known_by_peer[peer_id], block.transactions);
        for(std::map<long, Transaction>::iterator it_tx = block.transactions.begin(); it_tx != block.transactions.end(); it_tx++) {
          // Remove the confirmed tx from any possible object to request
          objects_ids_to_request.erase(objects_ids_to_request.find(it_tx->first));
        }
      }
    } else {
      XBT_INFO(
        "received a new block %ld from %d with %ld txs which doesn't represent a new best chain",
        block.id,
        relayed_by_peer_id,
        block.transactions.size()
      );
    }
  } else {
    // When a block arrives we only need to do something only if we didn't
    // know about it before.
    XBT_INFO(
      "received a known block %ld from %d with %ld transactions",
      block.id,
      relayed_by_peer_id,
      block.transactions.size()
    );
  }
  handle_orphan_blocks(block);
}

void Node::handle_orphan_blocks(Block block)
{
  // Check if this block is the parent of current orphan blocks
  if (orphan_blocks.find(block.id) != orphan_blocks.end()) {
    std::vector<Block> orphans = orphan_blocks[block.id];
    orphan_blocks[block.id].clear();
    for(std::vector<Block>::iterator it_orphan = orphans.begin(); it_orphan != orphans.end(); it_orphan++) {
      XBT_INFO(
        "found parent %ld for %ld",
        block.id,
        it_orphan->id
      );
      Block orphan = *it_orphan;
      handle_new_block(my_id, &orphan);
    }
  }
}

bool Node::blockchain_tip_updated(Block block)
{
  if (known_blocks_by_id.find(block.parent_id) == known_blocks_by_id.end()) {
    orphan_blocks[block.parent_id].push_back(block);
    XBT_INFO("received orphan block %ld", block.id);
    return false;
  }
  known_blocks_by_id.insert(std::make_pair(block.id, block));
  // Remove the received block from any possible object to request
  objects_ids_to_request.erase(objects_ids_to_request.find(block.id));
  // Check if we found a new best chain
  // Given that each block's diffulty is simulated we only need a way for every node to agree on a certain best chain
  // for our solution we achieve this concensus by comparing the randon block id and choosing the higher one
  if (block.id > blockchain_tip) {
    if (block.parent_id != blockchain_tip) {
      XBT_INFO("reorg_txs");
      reorg_txs(block.id, blockchain_tip);
    }
    blockchain_tip = block.id;
    blockchain_height = block.height;
    known_blocks_by_height.insert(std::make_pair(block.height, block));
    if ((blockchain_height % INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS) == 0) {
      double expected_time = INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS * INTERVAL_BETWEEN_BLOCKS_IN_SECONDS;
      // We add "- 1" to simulate the off-by-one bug error in the reference client implementation
      double base_time = known_blocks_by_height.find(blockchain_height - (INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS - 1))->second.time;
      double actual_time = block.time - base_time;
      long long new_difficulty = block.network_difficulty * expected_time / actual_time;
      XBT_INFO(
        "blockchain_height %d previous difficulty %lld, new difficulty %lld block date %lf expected time %lf actual time %lf",
        blockchain_height,
        difficulty,
        new_difficulty,
        block.time,
        expected_time,
        actual_time
      );
      difficulty = new_difficulty;
    }
    return true;
  } else {
    return false;
  }
}

// When a block reorganization occurs I need to "forget" about known transactions
// that had got confirmed in (now) orphaned blocks. Then I need to learn about the
// transactions that I didn't consider before because they belonged to orphaned blocks
void Node::reorg_txs(long new_tip_id, long old_tip_id)
{
  long common_parent_id = find_common_parent_id(new_tip_id, old_tip_id);
  XBT_INFO(
    "common_parent_id is %ld",
    common_parent_id
  );
  long current_block_id = old_tip_id;
  std::set<long> known_txs_to_discard;
  while (current_block_id != common_parent_id) {
    Block block = known_blocks_by_id.find(current_block_id)->second;
    known_txs_to_discard = JoinSets(known_txs_to_discard, JustKeys(block.transactions));
    current_block_id = block.parent_id;
  }
  known_txs_ids = DiffSets(known_txs_ids, known_txs_to_discard);
  current_block_id = new_tip_id;
  std::set<long> known_txs_to_add;
  while (current_block_id != common_parent_id) {
    Block block = known_blocks_by_id.find(current_block_id)->second;
    known_txs_to_add = JoinSets(known_txs_to_add, JustKeys(block.transactions));
    current_block_id = block.parent_id;
  }
  known_txs_ids = JoinSets(known_txs_ids, known_txs_to_add);
  XBT_INFO(
    "reorganizing blocks. known txs discarded %ld. known txs added %ld",
    DiffSets(known_txs_to_discard, known_txs_to_add).size(),
    DiffSets(known_txs_to_add, known_txs_to_discard).size()
  );
}

long Node::find_common_parent_id(long new_parent_tip_id, long old_parent_tip_id)
{
  std::set<long> parents_for_new_id = {new_parent_tip_id};
  std::set<long> parents_for_old_id = {old_parent_tip_id};
  while (IntersectSets(parents_for_new_id, parents_for_old_id).size() == 0) {
    new_parent_tip_id = known_blocks_by_id.find(new_parent_tip_id)->second.parent_id;
    old_parent_tip_id = known_blocks_by_id.find(old_parent_tip_id)->second.parent_id;
    parents_for_new_id.insert(new_parent_tip_id);
    parents_for_old_id.insert(old_parent_tip_id);
  }
  return *(IntersectSets(parents_for_new_id, parents_for_old_id).begin());
}

double Node::get_time_to_process_block(Block block)
{
  // Coefficients for f(x) = c2*x^2 + c1*x + c0; where x is the block size and f(x) the time to process it
  double c2 = 3.4510e-03;
  double c1 = -3.3800e-01;
  double c0 = 4.0727e+03;
  long x = block.size;
  double pre_processed_time = c2 * x * x + c1 * x + c0;
  double scale_factor = 1.16401e02;// We found out this is a good aproximation when comparing against the bitcoin reference client
  scale_factor = 1.16401e-02;
  return pre_processed_time * scale_factor;
}

void Node::handle_transactions(int relayed_by_peer_id, UnconfirmedTransactions *message)
{
  std::map<long, Transaction> txs_we_didnt_know = DiffMaps(message->unconfirmed_transactions, known_txs_ids);
  // Remove the received txs from any possible object to request
  objects_ids_to_request = DiffSets(objects_ids_to_request, JustKeys(txs_we_didnt_know));
  known_txs_ids = JoinMaps(known_txs_ids, message->unconfirmed_transactions);
  XBT_INFO(
    "received %ld unconfirmed transactions (%ld of them new) from %d",
    message->unconfirmed_transactions.size(),
    txs_we_didnt_know.size(),
    relayed_by_peer_id
  );
  // The unconfirmed transactions to broadcast will be the ones I didn't know of before
  txs_to_broadcast = JoinMaps(txs_to_broadcast, txs_we_didnt_know);
  // The unconfirmed transactions I'm aware of now include the ones I just received
  mempool = JoinMaps(mempool, message->unconfirmed_transactions);
  if (relayed_by_peer_id != my_id) {
    // Now I need to update the txs that I know my peer knows about
    txs_known_by_peer[relayed_by_peer_id] = JoinMaps(txs_known_by_peer[message->peer_id], message->unconfirmed_transactions);
  }
  // Fix: find a more suitable way to calculate execution duration for unconfirmed txs
  simgrid::s4u::this_actor::execute(1e8 * txs_we_didnt_know.size());// work for .1 seconds for each transaction
}

// Other peer is informing us about some inventory he knows about. If we don't know about some object we're
// going to request it from said peer
void Node::handle_inv(int relayed_by_peer_id, Inv *message)
{
    std::map<long, e_inv_type> objects_received = message->objects;
    for(std::map<long, e_inv_type>::iterator it_object = objects_received.begin(); it_object != objects_received.end(); it_object++) {
      // Add the objects we don't know about yet only if we are not already going to ask for it to another peer
      if (objects_ids_to_request.find(it_object->first) == objects_ids_to_request.end()) {
        switch (it_object->second) {
          case INV_BLOCK:
            XBT_INFO(
              "need to request block %ld from %d",
              it_object->first,
              relayed_by_peer_id
            );
            blocks_known_by_peer[relayed_by_peer_id].insert(it_object->first);
            if (known_blocks_by_id.find(it_object->first) == known_blocks_by_id.end()) {
              // I don't know about this block => I will ask the peer to send it to me
              objects_ids_to_request.insert(it_object->first);
              objects_to_request_from_peer[relayed_by_peer_id].insert(*it_object);
            }
            break;
          case INV_TX:
            XBT_INFO(
              "need to request tx %ld from %d",
              it_object->first,
              relayed_by_peer_id
            );
            txs_known_by_peer[relayed_by_peer_id].insert(it_object->first);
            if (known_txs_ids.find(it_object->first) == known_txs_ids.end()) {
              // I don't know about this tx => I will ask the peer to send it to me
              objects_ids_to_request.insert(it_object->first);
              objects_to_request_from_peer[relayed_by_peer_id].insert(*it_object);
            }
            break;
          default:
            THROW_IMPOSSIBLE;
        }
      }
    }
}

// Other peer is asking that we send him inventory that we know about
void Node::handle_getdata(int relayed_by_peer_id, GetData *message) {
  XBT_INFO("node %d requested %ld objects", relayed_by_peer_id, message->objects.size());
  objects_to_send_to_peer[relayed_by_peer_id] = JoinMaps(objects_to_send_to_peer[relayed_by_peer_id], message->objects);
}

long Node::compute_mempool_size()
{
  long result = 0;
  typename std::map<long, Transaction>::const_iterator it = mempool.begin();
  while (it != mempool.end()) {
    result += it->second.size;
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

int Node::on_exit(void*, void*)
{
  XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);
  XBT_INFO("shut down");
  return 0;
}
