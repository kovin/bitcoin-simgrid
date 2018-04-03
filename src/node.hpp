#ifndef NODE_HPP
#define NODE_HPP

#include "base_node.hpp"
#include "known_block.hpp"

class Node : public BaseNode
{
public:
  double msg_size = 1000000;

  explicit Node() {};
  explicit Node(std::vector<std::string> args);

protected:
  std::set<long> known_txs_ids;// set of transactions ids we know about
  std::map<long, Transaction> mempool;// map of unconfirmed transactions: <txid, tx>
  void init_from_args(std::vector<std::string> args);
  long blockchain_tip = 0;// The block id corresponding to the top of the best chain so far

  std::string get_node_data_filename(int id);
  void generate_activity();
  void send_messages();
  long compute_mempool_size();
  simgrid::s4u::MailboxPtr get_peer_incoming_mailbox(int peer_id);
  simgrid::s4u::MailboxPtr get_peer_outgoing_mailbox(int peer_id);
  void handle_new_block(int relayed_by_peer_id, Block *message);
  static int on_exit(void*, void*);

private:
  std::map<long, Block> blocks_to_broadcast;
  std::map<int, std::set<long>> blocks_known_by_peer;
  std::map<long, Transaction> txs_to_broadcast;
  std::map<int, std::set<long>> txs_known_by_peer;
  std::map<long, std::set<long>> utxo;// map of <txid, [o0, ..., on]> where oi is an unspent oupoint corresponding to the tx with id txid
  std::map<long, KnownBlock> known_blocks = {{0, KnownBlock(-1, 0, 0, {})}};// map of blocks we know about: <block-id, aggregated difficulty>
  double event_probability;
  int txs_per_day;
  double next_activity_time = 0;

  void do_set_next_activity_time();
  void process_messages();
  void handle_unconfirmed_transactions(int relayed_by_peer_id, UnconfirmedTransactions *message);
  double get_time_to_process_block(Block block);
  void send_blocks();
  void send_unconfirmed_transactions();
  bool blockchain_tip_updated(Block block);
  void reorg_txs(int new_tip_id, int old_tip_id);
  int find_common_parent_id(int new_tip_id, int old_tip_id);
};

#endif /* NODE_HPP */
