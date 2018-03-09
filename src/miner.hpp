#ifndef MINER_HPP
#define MINER_HPP

#include "node.hpp"

class Miner : public Node {
public:
  explicit Miner(std::vector<std::string> args) : Node(args) {}
protected:
  void send_messages();
  Message* get_message_to_send();
  void handle_new_transaction(Transaction *transaction);
};

#endif /* MINER_HPP */
