#include "miner.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

void Miner::send_messages()
{
  if ((messages_to_send > 0) && ((rand() % 100) < 5)) {
    Node::send_message_to_peers(get_message_to_send());
    messages_to_send--;
  }
}

Message* Miner::get_message_to_send()
{
  Block* block = new Block(my_id, blockchain_top, mempool);
  long previous_difficulty = known_blocks[blockchain_top];
  known_blocks[block->id] = block->difficulty + previous_difficulty;
  blockchain_top = block->id;
  long pre_size = compute_mempool_size();
  mempool = std::map<long, Transaction>();
  long post_size = compute_mempool_size();
  network_bytes_produced += (post_size - pre_size + block->size);
  return block;
}
