#include "node.hpp"
#include "aux-functions.hpp"

template<typename Out>
void split(const std::string &s, char delim, Out result)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
// </prueba>

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

int Node::active_nodes = 0;
long Node::network_bytes_produced = 0;

Node::Node(std::vector<std::string> args)
{
  active_nodes++;
  xbt_assert((args.size() - 1) == 3, "Expecting 3 parameters from the XML deployment file but got %zu", (args.size() - 1));
  my_id = std::stol(args[1]);
  peers_count = std::stol(args[2]);
  std::vector<std::string> peers;
  split(args[3], ' ', std::back_inserter(peers));
  std::vector<std::string>::iterator it;
  for(it = peers.begin(); it != peers.end(); it++) {
    int peer_id = std::stoi(it->c_str());
    my_peers.push_back(peer_id);
  }
  xbt_assert(peers_count > 0, "You should define at least one peer");
  std::string my_mailbox_name = std::string("receiver-") + std::to_string(my_id);
  my_mailbox = simgrid::s4u::Mailbox::byName(my_mailbox_name);
}

void Node::operator()()
{
  while (messages_to_send > 0) {
    receive();
    create_and_send_message_if_needed();
  }
  while (simgrid::s4u::Engine::getClock() < 1000) {
    receive();
    simgrid::s4u::this_actor::sleep_for(1);
  }
  /*XBT_INFO(
    "\nStats\n"
    "\ttotal bytes received:\t%d\n"
    "\ttotal network bytes produced:\t%d",
    total_bytes_received, network_bytes_produced
  );*/
  active_nodes--;
}

void Node::create_and_send_message_if_needed()
{
  if ((messages_to_send > 0) && ((rand() % 100) < 75)) {
    send_message_to_peers(get_message_to_send());
    messages_to_send--;
  }
}

void Node::notify_unconfirmed_transactions_if_needed()
{
  if ((rand() % 100) < 25) {
    Message *payload = new UnconfirmedTransactions(my_id, mempool);
    send_message_to_peers(payload);
  }
}

/*
TODO: no enviar a todos los peers
solo enviar a 1 peer todas las transacciones que conozcamos
al 25% restante enviar todo (excepto nuestras propias transacciones)
*/
void Node::send_message_to_peers(Message* payload)
{
  std::vector<int>::iterator it_id;
  for(it_id = my_peers.begin(); it_id != my_peers.end(); it_id++) {
      int peer_index_to_contact = *it_id;
      int peer_id = (peer_index_to_contact + my_id) % peers_count;
      simgrid::s4u::MailboxPtr mbox = get_peer_mailbox(peer_id);
      mbox->put_async(payload, msg_size + payload->size);
  }
}

Message* Node::get_message_to_send()
{
  long numberOfBytes = rand() & 100000;
  Message* message = new Transaction(my_id, numberOfBytes);
  total_bytes_received += message->size;
  network_bytes_produced += message->size;
  return message;
}

void Node::receive()
{
  if (!my_mailbox->empty()) {
    void* data = my_mailbox->get();
    Message *payload = static_cast<Message*>(data);
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
    notify_unconfirmed_transactions_if_needed();
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
  if (blockchain.find(block->id) == blockchain.end()) {
    blockchain.insert(block->id);
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