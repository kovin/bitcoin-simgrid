#ifndef NODE_HPP
#define NODE_HPP

#include "base_node.hpp"
#include "shared_data.hpp"
#include "validator_timer.hpp"
#include "../trace/trace_item.hpp"

/*
* This class represents a node (a miner is also a node with additional specialization) that knows how to:
* - create txs
* - receive messages from its peers (txs, blocks, inventories, request for inventories)
* - send the corresponding objects to their peers after a request for inventory
* - knows how to maintain a consitent blockchain every time a new block arrives, reorganizing txs if needed
* - has a mempool with unconfirmed txs from where it removes txs as they get confirmed in new received blocks
*/
class Node : public BaseNode
{
public:
  explicit Node() {};
  explicit Node(std::vector<std::string> args);
  double get_next_activity_time();

protected:
  // current network difficulty
  unsigned long long difficulty;
  // set of transactions ids we know about
  std::set<long> known_txs_ids;
  // map of unconfirmed transactions: <txid, tx>
  std::map<long, Transaction> mempool;
  // the block id corresponding to the top of the best chain so far
  long blockchain_tip = 0;
  // the block height corresponding to the top of the best chain so far
  int blockchain_height = 0;
  // map of blocks ids we know about indexed by their height in the blockchain
  std::map<int, long> known_blocks_ids_by_height = {{0, 0}};
  // set of blocks ids we know about
  std::set<long> known_blocks_ids = {0};

  // Will initialized the structures for this node by parsing the provided arguments
  void init_from_args(std::vector<std::string> args);
  // Will return the file with the deployment information for this particular node
  std::string get_node_data_filename(int id);
  // Will generate txs if it's a node or txs/blocks if it's a miner. As a precondition the current
  // time has to be at least the same as the next_activity_time
  void generate_activity();
  // For each peer will process at most 1 message from it and send any pending messages to it.
  // Returns true if it had work to do
  bool handle_messages();
  // Given the list of unconfirmed txs returns the size in bytes of that set
  long compute_mempool_size();
  // Returns the mailbox to send messages to a given peer
  simgrid::s4u::MailboxPtr get_peer_incoming_mailbox(int peer_id);
  // Returns the mailbox to receive messages from a given peer
  simgrid::s4u::MailboxPtr get_peer_outgoing_mailbox(int peer_id);
  // Handles a block relayed by relayed_by_peer_id. Returns true if it was a new block for this node.
  virtual bool handle_block(int relayed_by_peer_id, Block *message, bool force_broadcast = false);
  // Handles a blocks this node didn't know about
  void handle_new_block(int relayed_by_peer_id, const Block & block);
  // Handles the event when the tip of the blockchain needs to change with the provided block
  void handle_blockchain_tip_updated(int relayed_by_peer_id, const Block & block);
  // Creats a transaction object. It's called from the generate_activity() method
  Transaction create_transaction(long size, long fee_per_byte, double confirmed);
// Will handle the situation where the best blockchain will become that one identified by block
  bool blockchain_tip_updated(int relayed_by_peer_id, Block block);

private:
  // The ids of the blocks I received and that I know must be included in new inventory messages for my peers
  std::map<int, std::set<long>> blocks_ids_to_broadcast;
  // The blocks ids I know that my peers know about (so I don't notify them again about them)
  std::map<int, std::set<long>> blocks_known_by_peer;
  // The ids of txs I received and that I know must be included in new inventory messages for my peers
  std::map<int, std::set<long>> txs_ids_to_broadcast;
  // The txs ids I know my peers know about (so I don't notify them again about them)
  std::map<int, std::set<long>> txs_known_by_peer;
  // These are blocks that I received but for which I still don't know about their parents
  std::map<long, std::vector<Block>> orphan_blocks;
  // This is the set of ids (blocks ids or txs ids) that I need to request from my peers
  std::set<long> objects_to_request;
  // I keep a list of the objects ids I need to request from each peer
  std::map<int, std::set<long>> objects_to_request_from_peer;
  // I keep a list of object ids I need to send to each peer
  std::map<int, std::set<long>> objects_to_send_to_peer;
  // This is the next activity item that I will use to generate a tx and broadcast it to my peers
  TraceItem next_activity_item;
  // This is the time where I should generate the next transaction (based on next_activity_item)
  double next_activity_time = 0;
  // This is the auxiliary class in charge of computing the time we need to simulate when validating blocks and txs
  ValidatorTimer validator_timer;
  // Whether this node is enabled to produce transactions or not
  bool creates_txs;
  // Whether we should create txs following a model based on our hashreate the network difficulty
  // or based on a real blockchain trace
  bool using_trace;
  // If I'm generating the txs following a real blockchain trace, this is where I store the index of
  // the next acitivity item that need to be user to create the new tx
  int current_trace_index = 0;
  // If I'm generating the txs following a real blockchain trace, this is where I store the trace information
  std::vector<TraceItem> trace;

  // Checks fromt the logic peers of this node at most one message per each peer, process it, and returns
  // true if it processed at least one message
  bool receive_messages_from_peer(int peer_id);
  // Sends any pending message it may have to peer identified with id = peer_id
  void send_messages_to_peer(int peer_id);
  // Performs some housekeeping cleaning operations after a round of sending/receiving messages
  void cleanup(int peer_id);
  // Sets the simulation time when the next generation activity should happen for this node
  void do_set_next_activity_time();
  // Given a list of transactions, it process it and returns true if there was at least one we didn't know
  bool handle_transactions(int relayed_by_peer_id, Transactions *message);
  // Given an inventory message from one of its peers, it will check if something needs to be done (eg: sending a MESSAGE_GETDATA)
  void handle_inv(int relayed_by_peer_id, Inv *message);
  // Performs a block request to the peer identified by relayed_by_peer_id
  void request_block(int relayed_by_peer_id, long block_id);
  // Given a MESSAGE_GETDATA will register the request to then send the requested objects to the peer identified by relayed_by_peer_id
  void handle_getdata(int relayed_by_peer_id, GetData *message);
  // Will send any pending blocks to the peer identified with peer_id. These are blocks the peer didn't know about
  void send_blocks(int peer_id);
  // Will send any pending txs to the peer identified with peer_id. These are txs the peer didn't know about
  void send_transactions(int peer_id);
  // Will send a MESSAGE_INV to the peer identified with peer_id letting it know about some object it may not know about
  void inv(int peer_id);
  // Will send a MESSAGE_GETDATA to the peer identified with peer_id requesting some objects we need from it
  void getdata(int peer_id);
  // Will hanble blocks for which we don't know their parents
  void handle_orphan_blocks(Block block);
  // Will reorganize the confirmed/unconfirmed txs after a change in the blockchain tip
  void reorg_txs(long new_tip_id, long old_tip_id);
  // Given 2 blocks identifiers, it will return the most recent common parent for them
  long find_common_parent_id(long new_tip_id, long old_tip_id);
  // Every INTERVAL_BETWEEN_DIFFICULTY_RECALC_IN_BLOCKS we need to update network difficulty
  // We use the following function to check if we're in that situation and update the difficulty
  // accordingly
  void update_network_difficulty_if_needed(const Block & block);
};

#endif /* NODE_HPP */
