#ifndef TRACE_ITEM_MINER_HPP
#define TRACE_ITEM_MINER_HPP

#include "json.hpp"

using json = nlohmann::json;

struct TraceItemMiner {
  double received;
  double confirmed;
  std::string hash;
  long long difficulty;
  int n_tx;
  int n_tx_only_in_block;
};

void from_json(const json& j, TraceItemMiner& ttx);

#endif /* TRACE_ITEM_MINER_HPP */
