#ifndef NODE_HPP
#define NODE_HPP

#include "base_node.hpp"

class Node : public BaseNode
{
public:
  double msg_size = 1000000;

  explicit Node() {};
  explicit Node(std::vector<std::string> args);
  //void operator()();

protected:
  std::map<long, Transaction> mempool;// map of unconfirmed transactions: <txid, tx>
  std::map<long, std::set<long>> utxo;// map of <txid, [o0, ..., on]> where oi is an unspent oupoint corresponding to the tx with id txid
  long blockchain_top = 0;// The block id corresponding to the top of the best chain so far // FIXME: take this from json bootstrap data
  std::map<long, long> known_blocks = {{0, 100}};// map of blocks we know about: <block-id, aggregated difficulty>

  void init_from_args(std::vector<std::string> args);
  std::string get_node_data_filename(int id);
  void generate_activity();
  void send_messages();
  long compute_mempool_size();
  void handle_new_block(Block *block);
  void handle_unconfirmed_transactions(UnconfirmedTransactions *message);
  void send_message_to_peers(Message* payload, int percentage_of_peers);
  static int on_exit(void*, void*)
  {
    XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);
    XBT_DEBUG("shut down");
    return 0;
  }

private:
  double event_probability;
  int txs_per_day;
  double next_activity_time;

  void do_set_next_activity_time();
  void process_messages();
  void notify_unconfirmed_transactions_if_needed();
  simgrid::s4u::MailboxPtr get_peer_incoming_mailbox(int peer_id);
  simgrid::s4u::MailboxPtr get_peer_outgoing_mailbox(int peer_id);
  double get_time_to_process_block(Block* block);
};

#endif /* NODE_HPP */
