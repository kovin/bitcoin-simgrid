#ifndef CTG_HPP
#define CTG_HPP

#include "node.hpp"

using json = nlohmann::json;

class CTG
{
public:
  static simgrid::xbt::Extension<simgrid::s4u::Actor, CTG> EXTENSION_ID;

  explicit CTG();
  double get_next_activity_time(Node *node);

private:
    int nodes_count;
    int txs_per_day;
    bool usingModelMode;
    std::vector<double> event_probability;
    void compute_exponential_distribution(json ctg_data);
    void compute_uniform_distribution(json ctg_data);
};

#endif /* CTG_HPP */
