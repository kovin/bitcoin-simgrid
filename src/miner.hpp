#ifndef MINER_HPP
#define MINER_HPP

#include "node.hpp"

class Miner : public Node {
public:
  explicit Miner(std::vector<std::string> args) : Node(args) {}
protected:
  void create_and_send_message_if_needed();
  Message* get_message_to_send();
  void handle_new_transaction(Transaction *transaction);
};

#endif /* MINER_HPP */
