#ifndef MINER_HPP
#define MINER_HPP

#include "node.hpp"

class Miner : public Node {
public:
  explicit Miner(std::vector<std::string> args);

protected:
double get_next_activity_time();
  void init_from_args(std::vector<std::string> args);
  void generate_activity();

private:
  long long hashrate;
  double next_activity_time;

  void do_set_next_activity_time();
  double get_event_probability(int timespan);
};

#endif /* MINER_HPP */
