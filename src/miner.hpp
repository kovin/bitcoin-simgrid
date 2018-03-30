#ifndef MINER_HPP
#define MINER_HPP

#include "node.hpp"

class Miner : public Node {
public:
  explicit Miner(std::vector<std::string> args);

protected:
  void init_from_args(std::vector<std::string> args);
  void generate_activity();
  void send_messages();

private:
  long long difficulty;
  long long hashrate;
  double next_activity_time;
  Block* pending_block = NULL;

  // We set next time accoriding to the probability of this miner finding a block in the next 10 minutes
  void do_set_next_activity_time();
};

#endif /* MINER_HPP */
