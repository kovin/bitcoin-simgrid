#ifndef NODE_HPP
#define NODE_HPP

#include "base_node.hpp"

class Node : public BaseNode
{
public:
  explicit Node() {};
  explicit Node(std::vector<std::string> args);
  double get_next_activity_time();

protected:
  long long difficulty;// current network difficulty
  std::set<long> known_txs_ids;// set of transactions ids we know about
  std::map<long, Transaction> mempool;// map of unconfirmed transactions: <txid, tx>
  void init_from_args(std::vector<std::string> args);
  long blockchain_tip = 0;// The block id corresponding to the top of the best chain so far
  int blockchain_height = 0;// The block height corresponding to the top of the best chain so far
  std::map<int, Block> known_blocks_by_height = {{0, Block()}};// map of blocks we know about: <block-height, aggregated difficulty>
  std::map<long, Block> known_blocks_by_id = {{0, Block()}};// map of blocks we know about: <block-id, aggregated difficulty>

  std::string get_node_data_filename(int id);
  void generate_activity();
  void send_messages();
  long compute_mempool_size();
  simgrid::s4u::MailboxPtr get_peer_incoming_mailbox(int peer_id);
  simgrid::s4u::MailboxPtr get_peer_outgoing_mailbox(int peer_id);
  void handle_new_block(int relayed_by_peer_id, Block *message);
  Transaction create_transaction();
  static int on_exit(void*, void*);

private:
  std::map<long, Block> blocks_to_broadcast;
  std::map<int, std::set<long>> blocks_known_by_peer;
  std::map<long, Transaction> txs_to_broadcast;
  std::map<int, std::set<long>> txs_known_by_peer;
  std::map<long, std::set<long>> utxo;// map of <txid, [o0, ..., on]> where oi is an unspent oupoint corresponding to the tx with id txid
  std::map<long, std::vector<Block>> orphan_blocks;
  std::set<long> objects_ids_to_request;
  std::map<int, std::map<long, e_inv_type>> objects_to_request_from_peer;
  std::map<int, std::map<long, e_inv_type>> objects_to_send_to_peer;
  double next_activity_time = 0;

  void do_set_next_activity_time();
  void process_messages();
  void handle_transactions(int relayed_by_peer_id, UnconfirmedTransactions *message);
  void handle_inv(int relayed_by_peer_id, Inv *message);
  void handle_getdata(int relayed_by_peer_id, GetData *message);
  double get_time_to_process_block(Block block);
  void send_blocks();
  void send_transactions();
  void inv();
  void getdata();
  void handle_orphan_blocks(Block block);
  bool blockchain_tip_updated(Block block);
  void reorg_txs(long new_tip_id, long old_tip_id);
  long find_common_parent_id(long new_tip_id, long old_tip_id);
};

#endif /* NODE_HPP */
