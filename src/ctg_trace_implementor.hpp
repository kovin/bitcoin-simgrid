#ifndef CTG_TRACE_IMPLEMENTOR_HPP
#define CTG_TRACE_IMPLEMENTOR_HPP

#include "trace_item.hpp"
#include "ctg_base_implementor.hpp"

using json = nlohmann::json;

class CTG_TraceImplementor : public CTG_BaseImplementor
{
public:
  explicit CTG_TraceImplementor(json ctg_data);
  double get_next_activity_time(Node *node);

private:
    int current_trace_index = 0;
    std::vector<TraceItem> trace;
};

#endif /* CTG_TRACE_IMPLEMENTOR_HPP */
