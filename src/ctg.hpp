#ifndef CTG_HPP
#define CTG_HPP

#include "node.hpp"

using json = nlohmann::json;

class CTG
{
public:
  static simgrid::xbt::Extension<simgrid::s4u::Actor, CTG> EXTENSION_ID;

  explicit CTG()
  {
    std::string ctg_data_filename = deployment_directory + "ctg_data";
    std::ifstream ctg_data_stream(ctg_data_filename);
    json ctg_data;
    ctg_data_stream >> ctg_data;
    double nodes_count = ctg_data["nodes_count"].get<double>();
    xbt_assert(nodes_count > 0, "The amount of nodes should be strictly positive");
    txs_per_day = ctg_data["txs_per_day"].get<int>();
    xbt_assert(txs_per_day >= 0, "Transaction per day can't be negative");
    event_probability = 1 / nodes_count;
    XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);
    XBT_DEBUG("event probability is %f", event_probability);
  }

  double get_next_activity_time(Node *node)
  {
    // Between the moment we set the previous next_activity_time, we checked it and generated the
    // corresponding activity, we wasted some time that we need to substract from next_activity_time
    // if we expect to accomplish the number of txs_per_day
    double wasted_time = simgrid::s4u::Engine::getClock() - node->get_next_activity_time();
    return calc_next_activity_time(event_probability, 24 * 60 * 60, txs_per_day) - wasted_time;
  }
private:
    double event_probability;
    int txs_per_day;
};

#endif /* CTG_HPP */
