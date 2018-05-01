#include <random>
#include "ctg.hpp"
#include "bitcoin_simgrid.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

CTG::CTG()
{
  std::string ctg_data_filename = deployment_directory + "ctg_data";
  std::ifstream ctg_data_stream(ctg_data_filename);
  json ctg_data;
  ctg_data_stream >> ctg_data;
  nodes_count = ctg_data["nodes_count"].get<int>();
  xbt_assert(nodes_count > 0, "The amount of nodes should be strictly positive");
  usingModelMode = ctg_data["mode"].get<std::string>() == "model";
  txs_per_day = ctg_data["txs_per_day"].get<int>();
  xbt_assert(txs_per_day >= 0, "Transaction per day can't be negative");
  if (ctg_data["distribution"]["type"].get<std::string>() == "exponential") {
    compute_exponential_distribution(ctg_data);
  } else {
    compute_uniform_distribution(ctg_data);
  }
}

double CTG::get_next_activity_time(Node *node)
{
  // Between the moment we set the previous next_activity_time, we checked it and generated the
  // corresponding activity, we wasted some time that we need to substract from next_activity_time
  // if we expect to accomplish the number of txs_per_day
  double wasted_time = simgrid::s4u::Engine::getClock() - node->get_next_activity_time();
  return calc_next_activity_time(event_probability[node->get_id()], 24 * 60 * 60, txs_per_day) - wasted_time;
}

void CTG::compute_exponential_distribution(json ctg_data)
{
  XBT_DEBUG("event probability is exponential with lambda %f", ctg_data["distribution"]["lambda"].get<double>());
  const int nrolls = 1000 * nodes_count; // number of experiments
  std::default_random_engine generator;
  std::exponential_distribution<double> distribution(ctg_data["distribution"]["lambda"].get<double>());
  double p[nodes_count] = {};
  XBT_DEBUG("por aca 1");
  for (int i = 0; i < nrolls; ++i) {
    double number = distribution(generator);
    if (number < 1.0) {
      ++p[int(nodes_count * number)];
    }
  }
  XBT_DEBUG("por aca 2");
  for (int i = 0; i < nodes_count; ++i) {
    event_probability.push_back(p[i] / nrolls);
    XBT_DEBUG("event probability for node %d is %f", i, event_probability[i]);
  }
}

void CTG::compute_uniform_distribution(json ctg_data)
{
  for (int i = 0; i < nodes_count; i++) {
    event_probability.push_back(1.0 / nodes_count);
  }
  XBT_DEBUG("event probability is uniform and is %f for every node", 1.0 / nodes_count);
}
