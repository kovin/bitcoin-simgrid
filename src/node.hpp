#ifndef NODE_HPP
#define NODE_HPP
#define SLEEP_DURATION 1

#include "simgrid/s4u.hpp"
#include "message.hpp"
#include <cstdlib>
#include <set>
#include <iostream>
#include <string>
#include <sstream>

class BaseNode
{
protected:
  virtual void create_and_send_message_if_needed() = 0;
  virtual Message* get_message_to_send() = 0;
  virtual void handle_new_transaction(Transaction *transaction) = 0;
};

class Node : public BaseNode
{
  int peers_count;
  double msg_size = 1000000;
  std::vector<int> my_peers;
  simgrid::s4u::CommPtr comm_received = nullptr;
  simgrid::s4u::MailboxPtr my_mailbox;

public:
  explicit Node(std::vector<std::string> args);
  static int active_nodes;
  static long network_bytes_produced;
  void operator()();

protected:
  int my_id;
  double messages_to_send = 20;// FIXME: remove this
  std::map<long, Transaction> mempool;// map of unconfirmed transactions: <txid, tx>
  std::map<long, std::set<long>> utxo;// map of <txid, [o0, ..., on]> where oi is an unspent oupoint corresponding to the tx with id txid
  long blockchain_top = 0;// The block id corresponding to the top of the best chain so far // FIXME: take this from json bootstrap data
  std::map<long, long> known_blocks = {{0, 100}};// map of blocks we know about: <block-id, aggregated difficulty>

  void create_and_send_message_if_needed();
  Message* get_message_to_send();
  long compute_mempool_size();
  long total_bytes_received = 0;
  void handle_new_transaction(Transaction *transaction);
  void handle_new_block(Block *block);
  void handle_unconfirmed_transactions(UnconfirmedTransactions *message);
  void send_message_to_peers(Message* payload);

private:
  bool shutting_down = false;
  void receive();
  void notify_unconfirmed_transactions_if_needed();
  simgrid::s4u::MailboxPtr get_peer_mailbox(int peer_id);
  void wait_for_other_before_shutdown();
};

#endif /* NODE_HPP */
