#include "node.hpp"

using json = nlohmann::json;

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

Node::Node(std::vector<std::string> args)
{
  init_from_args(args);
  simgrid::s4u::this_actor::onExit((int_f_pvoid_pvoid_t) on_exit, NULL);
}

void Node::init_from_args(std::vector<std::string> args)
{
  BaseNode::init_from_args(args);
  event_probability = node_data["event_probability"].get<double>();
  xbt_assert(event_probability >= 0 && event_probability <= 1, "Probability of an event should be in the range [0, 1]");
  txs_per_day = node_data["txs_per_day"].get<int>();
  xbt_assert(txs_per_day >= 0, "Transaction per day can't be negative");
  do_set_next_activity_time();
}

std::string Node::get_node_data_filename(int id) {
  return deployment_directory + simgrid::s4u::this_actor::getName() + std::string("_data-") + std::to_string(my_id);
}

void Node::do_set_next_activity_time()
{
  XBT_DEBUG("event_probability: %f, txs_per_day: %d", event_probability, txs_per_day);
  next_activity_time = get_next_activity_time(event_probability, 24 * 60 * 60, txs_per_day);
}

/* We won't relay any messages in the next iteration unless:
* a) this node creates a tx
* b) we receive a block we didn't know about
* c) we receive unconfirmed txs that are not present in our mempool
*/
void Node::send_messages()
{
  // We will let each peer know about new blocks (but we won't send the blocks that we know the peer already knows)
  for(std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    std::map<long, Block> blocks_to_send = DiffMaps(blocks_to_broadcast, blocks_known_by_peer[peer_id]);
    typename std::map<long, Block>::const_iterator it_block = blocks_to_send.begin();
    while(it_block != blocks_to_send.end()) {
      XBT_DEBUG("sending block to %d", peer_id);
      simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
      mbox->put_async(new Block(it_block->second), msg_size + it_block->second.size);
      ++it_block;
    }
    // Note: we never clear the blocks we know each peer knows about
  }
  typename std::map<long, Block>::const_iterator it_block = blocks_to_broadcast.begin();
  while(it_block != blocks_to_broadcast.end()) {
    // Remove from txs_to_broadcast the ones that got confirmed in the current block
    txs_to_broadcast = DiffMaps(txs_to_broadcast, it_block->second.transactions);
    ++it_block;
  }
  blocks_to_broadcast.clear();
  // We will let each peer know about recent unconfirmed txs (but we won't send the txs that we know the peer already knows)
  for(std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    std::map<long, Transaction> txs_to_send = DiffMaps(txs_to_broadcast, txs_known_by_peer[peer_id]);
    if (txs_to_send.size() > 0) {
      Message *message = new UnconfirmedTransactions(my_id, txs_to_send);
      XBT_DEBUG("sending %ld unconfirmed transactions to %d", txs_to_send.size(), peer_id);
      simgrid::s4u::MailboxPtr mbox = get_peer_outgoing_mailbox(peer_id);
      mbox->put_async(message, msg_size + message->size);
    }
  }
  txs_to_broadcast.clear();
}

void Node::generate_activity()
{
  if (next_activity_time > simgrid::s4u::Engine::getClock()) {
    return;
  }
  do_set_next_activity_time();
  XBT_DEBUG("creating tx");
  long numberOfBytes = rand() & 100000;
  // FIXME: agregar datos del utxo que estamos gastando con esta transaccion.
  // el nodo deberia tener en su blockchain_data.json los datos de sus propios utxos
  std::map<long, Transaction> txs;
  txs.insert(std::make_pair(my_id, Transaction(my_id, numberOfBytes)));
  handle_unconfirmed_transactions(my_id, new UnconfirmedTransactions(my_id, txs));
}

void Node::process_messages()
{
  for(std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
    int peer_id = *it_id;
    simgrid::s4u::MailboxPtr mbox = get_peer_incoming_mailbox(peer_id);
    while (!mbox->empty()) {
      void* data = mbox->get();
      Message *payload = static_cast<Message*>(data);
      XBT_DEBUG("received %s from %d", payload->get_type_name().c_str(), payload->peer_id);
      switch (payload->get_type()) {
        case MESSAGE_BLOCK:
          handle_new_block(peer_id, static_cast<Block*>(data));
          break;
        case UNCONFIRMED_TRANSACTIONS:
          handle_unconfirmed_transactions(peer_id, static_cast<UnconfirmedTransactions*>(data));
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
  try {
    // When a block arrives we only need to do something only if we didn't
    // know about it before. So if this throws an out of range exception we
    // have work to do :)
    known_blocks.at(block.id);
  } catch (const std::out_of_range& oor) {
    if (relayed_by_peer_id != my_id) {
      blocks_known_by_peer[relayed_by_peer_id].insert(block.id);
      simgrid::s4u::this_actor::execute(get_time_to_process_block(block));
    }
    long previous_difficulty = known_blocks[blockchain_top];
    known_blocks[block.id] = block.difficulty + previous_difficulty;
    blocks_to_broadcast.insert(std::make_pair(block.id, block));
    // Remove from the unconfirmed transactions known by our peers those confirmed in the block we just received
    for(std::vector<int>::iterator it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
      int peer_id = *it_id;
      txs_known_by_peer[peer_id] = DiffMaps(txs_known_by_peer[peer_id], block.transactions);
    }
    // Now that we know of txs that got confirmed we need to evict them from our mempool
    mempool = DiffMaps(mempool, block.transactions);
  }
}

// TODO: deberia hacer el set del nuevo top de la blockchain en un metodo de la clase Node, tipo set_new_tip(Block block)
  // , en ese mismo metodo debería recalcular la dificultad actual si el bloque es multiplo de 2016
  //long previous_difficulty = known_blocks[blockchain_top];
  //known_blocks[block->id] = block->difficulty + previous_difficulty;
  //blockchain_top = block->id;

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

void Node::handle_unconfirmed_transactions(int relayed_by_peer_id, UnconfirmedTransactions *message)
{
  // The unconfirmed transactions to broadcast will be the ones I didn't know of before
  txs_to_broadcast = JoinMaps(txs_to_broadcast, DiffMaps(message->unconfirmed_transactions, mempool));
  // The unconfirmed transaction I'm aware of now include the ones I just received
  mempool = JoinMaps(mempool, message->unconfirmed_transactions);
  if (relayed_by_peer_id != my_id) {
    txs_known_by_peer[relayed_by_peer_id] = JoinMaps(txs_known_by_peer[message->peer_id], message->unconfirmed_transactions);
  }
  // Fix: find a more suitable way to calculate execution duration for unconfirmed txs
  simgrid::s4u::this_actor::execute(1e8 * message->unconfirmed_transactions.size());// work for .1 seconds for each transaction
}

long Node::compute_mempool_size()
{
  long result = 0;
  typename std::map<long, Transaction>::const_iterator it = mempool.begin();
  while (it != mempool.end())
  {
    result += it->second.size;
    ++it;
  }
  return result;
}

simgrid::s4u::MailboxPtr Node::get_peer_incoming_mailbox(int peer_id)
{
  std::string mboxName = std::string("from:") + std::to_string(peer_id) + "-to:" + std::to_string(my_id);
  return simgrid::s4u::Mailbox::byName(mboxName);
}

simgrid::s4u::MailboxPtr Node::get_peer_outgoing_mailbox(int peer_id)
{
  std::string mboxName = std::string("from:") + std::to_string(my_id) + "-to:" + std::to_string(peer_id);
  return simgrid::s4u::Mailbox::byName(mboxName);
}
