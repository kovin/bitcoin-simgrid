#include "trace_item_miner.hpp"

using json = nlohmann::json;

void from_json(const json& j, TraceItemMiner& ttx)
{
  ttx.received = j.at("received").get<double>();
  ttx.confirmed = j.at("confirmed").get<double>();
  ttx.hash = j.at("hash").get<std::string>();
  ttx.difficulty = j.at("difficulty").get<unsigned long long>();
  ttx.n_tx = j.at("n_tx").get<double>();
  ttx.txs_broadcasted_in_block = j.at("txs_broadcasted_in_block").get<std::vector<std::pair<long, long>>>();
}
