#include "base_node.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

void BaseNode::operator()()
{
while (simgrid::s4u::Engine::getClock() < SIMULATION_DURATION) {
  generate_activity();
  process_messages();
  send_messages();
  simgrid::s4u::this_actor::sleep_for(SLEEP_DURATION);
}
XBT_DEBUG("shutting down");
simgrid::s4u::Actor::killAll();
}

void BaseNode::init_from_args(std::vector<std::string> args)
{
  xbt_assert((args.size() - 1) == 1, "Expecting 1 parameter from the XML deployment file but got %zu", (args.size() - 1));
  my_id = std::stoi(args[1]);
  std::string node_data_filename = get_node_data_filename(my_id);
  std::ifstream node_data_stream(node_data_filename);
  xbt_assert(node_data_stream.good(), "File %s doesn't exist or the program doesn't have permission to read it", node_data_filename.c_str());
  node_data_stream >> node_data;
  my_peers = node_data["peers"].get<std::vector<int>>();
  xbt_assert(my_peers.size() > 0, "You should define at least one peer");
}

double BaseNode::get_next_activity_time(double probability, int timespan, int events_per_timespan)
{
  return simgrid::s4u::Engine::getClock() + (-log(1 - frand()) / probability) * timespan / events_per_timespan;
}
