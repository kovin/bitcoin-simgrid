#include <random>
#include "ctg_model_implementor.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

CTG_ModelImplementor::CTG_ModelImplementor(json ctg_data)
{
  nodes = ctg_data["nodes"].get<std::vector<int>>();
  xbt_assert(nodes.size() > 0, "The amount of nodes should be strictly positive");
  txs_per_day = ctg_data["distribution"]["txs_per_day"].get<int>();
  xbt_assert(txs_per_day >= 0, "Transactions per day can't be negative");
  if (ctg_data["distribution"]["type"].get<std::string>() == "exponential") {
    compute_exponential_distribution(ctg_data);
  } else {
    compute_uniform_distribution(ctg_data);
  }
}

TraceItem CTG_ModelImplementor::get_next_activity_item(Node *node)
{
  double next_activity_time = calc_next_activity_time(node->get_next_activity_time(), event_probability[node->get_id()], 24 * 60 * 60, txs_per_day);
  long size = lrand(AVERAGE_BYTES_PER_TX * 2);// On average txs size will be AVERAGE_BYTES_PER_TX bytes
  long fee_per_byte = lrand(AVERAGE_FEE_PER_BYTE * 2);// On average txs size will be AVERAGE_FEE_PER_BYTE bytes
  TraceItem trace_item = {
    received: next_activity_time,
    confirmed: next_activity_time,
    hash: "",
    size: size,
    fee_per_byte: fee_per_byte
  };
  return trace_item;
}

void CTG_ModelImplementor::compute_exponential_distribution(json ctg_data)
{
  LOG("event probability is exponential with lambda %f", ctg_data["distribution"]["lambda"].get<double>());
  const int nrolls = 1000 * nodes.size(); // number of experiments
  std::default_random_engine generator;
  std::exponential_distribution<double> distribution(ctg_data["distribution"]["lambda"].get<double>());
  double p[nodes.size()] = {};
  for (int i = 0; i < nrolls; ++i) {
    double number = distribution(generator);
    if (number < 1.0) {
      int node_id = nodes[int(nodes.size() * number)];
      ++p[node_id];
    }
  }
  for (int i = 0; i < nodes.size(); ++i) {
    event_probability.push_back(p[i] / nrolls);
    LOG("event probability for node %d is %f", i, event_probability[i]);
  }
}

void CTG_ModelImplementor::compute_uniform_distribution(json ctg_data)
{
  for (int i = 0; i < nodes.size(); i++) {
    event_probability.push_back(1.0 / nodes.size());
  }
  LOG("event probability is uniform and is %f for every node", 1.0 / nodes.size());
}
