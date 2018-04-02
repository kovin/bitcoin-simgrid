#ifndef NODE_HPP
#define NODE_HPP

#include "base_node.hpp"

class Node : public BaseNode
{
public:
  double msg_size = 1000000;

  explicit Node() {};
  explicit Node(std::vector<std::string> args);

protected:
  std::map<long, Transaction> mempool;// map of unconfirmed transactions: <txid, tx>
  long blockchain_top = 0;// The block id corresponding to the top of the best chain so far // FIXME: take this from json bootstrap data

  void init_from_args(std::vector<std::string> args);
  std::string get_node_data_filename(int id);
  void generate_activity();
  void send_messages();
  long compute_mempool_size();
  static int on_exit(void*, void*)
  {
    XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);
    XBT_DEBUG("shut down");
    return 0;
  }
  simgrid::s4u::MailboxPtr get_peer_incoming_mailbox(int peer_id);
  simgrid::s4u::MailboxPtr get_peer_outgoing_mailbox(int peer_id);
  void handle_new_block(int relayed_by_peer_id, Block *message);

private:
  std::map<long, Block> blocks_to_broadcast;
  std::map<int, std::set<long>> blocks_known_by_peer;
  std::map<long, Transaction> txs_to_broadcast;
  std::map<int, std::set<long>> txs_known_by_peer;
  std::map<long, std::set<long>> utxo;// map of <txid, [o0, ..., on]> where oi is an unspent oupoint corresponding to the tx with id txid
  std::map<long, long> known_blocks = {{0, 100}};// map of blocks we know about: <block-id, aggregated difficulty>
  double event_probability;
  int txs_per_day;
  double next_activity_time = 0;

  void do_set_next_activity_time();
  void process_messages();
  void handle_unconfirmed_transactions(int relayed_by_peer_id, UnconfirmedTransactions *message);
  double get_time_to_process_block(Block block);
  void send_blocks();
  void send_unconfirmed_transactions();
};

#endif /* NODE_HPP */
