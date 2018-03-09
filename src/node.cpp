#include "bitcoin-simgrid.hpp"
#include "node.hpp"
#include "aux-functions.hpp"
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

int Node::active_nodes = 0;
int Node::messages_produced = 0;
int Node::messages_received = 0;
long Node::network_bytes_produced = 0;

Node::Node(std::vector<std::string> args)
{
  active_nodes++;
  init_from_args(args);
  std::string my_mailbox_name = std::string("receiver-") + std::to_string(my_id);
  my_mailbox = simgrid::s4u::Mailbox::byName(my_mailbox_name);
  simgrid::s4u::this_actor::onExit((int_f_pvoid_pvoid_t) on_exit, NULL);
}

void Node::init_from_args(std::vector<std::string> args)
{
  xbt_assert((args.size() - 1) == 1, "Expecting 1 parameter from the XML deployment file but got %zu", (args.size() - 1));
  my_id = std::stoi(args[1]);
  std::string node_data_filename = deployment_directory + std::string("/node_data-") + std::to_string(my_id);
  std::ifstream node_data_stream(node_data_filename);
  xbt_assert(node_data_stream.good(), "File %s doesn't exist or the program doesn't have permission to read it", node_data_filename.c_str());
  json node_data;
  node_data_stream >> node_data;
  my_peers = node_data["peers"].get<std::vector<int>>();
  xbt_assert(my_peers.size() > 0, "You should define at least one peer");
}

void Node::operator()()
{
  while (messages_to_send > 0) {
    process_messages();
    send_messages();
  }
  wait_for_others_before_shutdown();
}

void Node::wait_for_others_before_shutdown()
{
  XBT_DEBUG("shutting down");
  shutting_down = true;
  while (!my_mailbox->empty()) {
    process_messages();
    simgrid::s4u::this_actor::sleep_for(SLEEP_DURATION);
  }
  active_nodes--;
  while (messages_received < messages_produced) {
    process_messages();
    simgrid::s4u::this_actor::sleep_for(SLEEP_DURATION);
  }
  while (active_nodes > 0) {
    simgrid::s4u::this_actor::sleep_for(SLEEP_DURATION);
  }
  XBT_DEBUG("killing others");
  simgrid::s4u::Actor::killAll();
}

void Node::send_messages()
{
  if ((messages_to_send > 0) && ((rand() % 100) < 75)) {
    send_message_to_peers(get_message_to_send());
    messages_to_send--;
  }
}

/*
FIXME: no enviar mis transacciones a todos los peers
solo enviar a 1 peer todas las transacciones que conozcamos
al 25% restante enviar todo (excepto nuestras propias transacciones)
*/
void Node::send_message_to_peers(Message* payload)
{
  std::vector<int>::iterator it_id;
  for(it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
      int peer_id = *it_id;
      XBT_DEBUG("sending %s to %d", payload->get_type_name().c_str(), peer_id);
      simgrid::s4u::MailboxPtr mbox = get_peer_mailbox(peer_id);
      messages_produced++;
      mbox->put_async(payload, msg_size + payload->size);
  }
}

Message* Node::get_message_to_send()
{
  long numberOfBytes = rand() & 100000;
  // FIXME: agregar datos del utxo que estamos gastando con esta transaccion.
  // el nodo deberia tener en su blockchain_data.json los datos de sus propios utxos
  Message* message = new Transaction(my_id, numberOfBytes);
  total_bytes_received += message->size;
  network_bytes_produced += message->size;
  return message;
}

void Node::process_messages()
{
  if (my_mailbox->empty()) {
    return;
  }
  while (!my_mailbox->empty()) {
    messages_received++;
    void* data = my_mailbox->get();
    Message *payload = static_cast<Message*>(data);
    XBT_DEBUG("received %s from %d", payload->get_type_name().c_str(), payload->peer_id);
    comm_received = nullptr;
    switch (payload->get_type()) {
      case MESSAGE_TRANSACTION:
        handle_new_transaction(static_cast<Transaction*>(data));
        break;
      case MESSAGE_BLOCK:
        handle_new_block(static_cast<Block*>(data));
        break;
      case UNCONFIRMED_TRANSACTIONS:
        handle_unconfirmed_transactions(static_cast<UnconfirmedTransactions*>(data));
        break;
      default:
        THROW_IMPOSSIBLE;
    }
  }
  notify_unconfirmed_transactions_if_needed();
}

void Node::notify_unconfirmed_transactions_if_needed()
{
  if (shutting_down) {
    return;
  }
  if ((rand() % 100) < 25) {
    Message *payload = new UnconfirmedTransactions(my_id, mempool);
    send_message_to_peers(payload);
  }
}

void Node::handle_new_transaction(Transaction *transaction)
{
  std::map<long, Transaction>::iterator it;
  it = mempool.find(transaction->id);
  if (it == mempool.end()) {
    long pre_size = compute_mempool_size();
    mempool.insert(std::make_pair(transaction->id, *transaction));
    long post_size = compute_mempool_size();
    total_bytes_received += (post_size - pre_size);
    network_bytes_produced += (post_size - pre_size);
  }
}

void Node::handle_new_block(Block *block)
{
  try {
    known_blocks.at(block->id);
  } catch (const std::out_of_range& oor) {
    long previous_difficulty = known_blocks[blockchain_top];
    known_blocks[block->id] = block->difficulty + previous_difficulty;
    long pre_size = compute_mempool_size();
    mempool = DiffMaps(mempool, block->transactions);
    long post_size = compute_mempool_size() + block->size;
    total_bytes_received += (post_size - pre_size);
    network_bytes_produced += (post_size - pre_size);
  }
}

void Node::handle_unconfirmed_transactions(UnconfirmedTransactions *message)
{
  long pre_size = compute_mempool_size();
  mempool = JoinMaps(mempool, message->unconfirmed_transactions);
  long post_size = compute_mempool_size();
  total_bytes_received += (post_size - pre_size);
  network_bytes_produced += (post_size - pre_size);
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

simgrid::s4u::MailboxPtr Node::get_peer_mailbox(int peer_id)
{
  std::string mboxName = std::string("receiver-") + std::to_string(peer_id);
  return simgrid::s4u::Mailbox::byName(mboxName);
}
