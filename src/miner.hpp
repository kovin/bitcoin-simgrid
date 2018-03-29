#ifndef MINER_HPP
#define MINER_HPP

#include "node.hpp"

class Miner : public Node {
public:
  explicit Miner(std::vector<std::string> args);
/*public:
  explicit Miner(std::vector<std::string> args) {
    init_from_args(args);
    simgrid::s4u::this_actor::onExit((int_f_pvoid_pvoid_t) on_exit, NULL);
  }*/

protected:
  void init_from_args(std::vector<std::string> args);
  void generate_activity();
  void send_messages();

private:
  long long difficulty;
  long long hashrate;
  double next_activity_time;
  Block* pending_block = NULL;

  void do_set_next_activity_time();
};

#endif /* MINER_HPP */
