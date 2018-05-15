#include "ctg_trace_implementor.hpp"


XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

CTG_TraceImplementor::CTG_TraceImplementor(json ctg_data)
{
  trace = ctg_data["trace"].get<std::vector<TraceItem>>();
}

double CTG_TraceImplementor::get_next_activity_time(Node *node)
{
  if (current_trace_index < trace.size()) {
    return trace[current_trace_index++].received;
  } else {
    // No more txs to distribute among nodes => return SIMULATION_DURATION to avoid creating more tx activity during this simulation
    return SIMULATION_DURATION;
  }
}
