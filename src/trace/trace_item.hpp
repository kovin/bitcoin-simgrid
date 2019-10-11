#ifndef TRACE_ITEM_HPP
#define TRACE_ITEM_HPP

#include "../json.hpp"

using json = nlohmann::json;

struct TraceItem {
  double received;
  double confirmed;
  std::string hash;
  long size;
  long fee_per_byte;
};

void from_json(const json& j, TraceItem& ttx);

#endif /* TRACE_ITEM_HPP */
