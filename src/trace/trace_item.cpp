#include "trace_item.hpp"

using json = nlohmann::json;

void from_json(const json& j, TraceItem& ttx)
{
  ttx.received = j.at("received").get<double>();
  ttx.confirmed = j.at("confirmed").get<double>();
  ttx.hash = j.at("hash").get<std::string>();
  ttx.size = j.at("size").get<long>();
  ttx.fee_per_byte = j.at("fee_per_byte").get<long>();
}
