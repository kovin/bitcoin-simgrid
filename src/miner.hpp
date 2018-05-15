#ifndef MINER_HPP
#define MINER_HPP

#include "node.hpp"
#include "trace_item.hpp"

class Miner : public Node {
public:
  explicit Miner(std::vector<std::string> args);
  double get_next_activity_time();

protected:
  void init_from_args(std::vector<std::string> args);
  void generate_activity();

private:
  bool using_trace;
  int current_trace_index = 0;
  std::vector<TraceItem> trace;
  long long hashrate;
  double next_activity_time;

  void do_set_next_activity_time();
  double get_event_probability(int timespan);
};

#endif /* MINER_HPP */
