#include "base_node.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

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

void BaseNode::operator()()
{
  while (simgrid::s4u::Engine::get_clock() < SIMULATION_DURATION) {
    generate_activity();
    process_messages();
    send_messages();
    double sleep_duration = std::min(SLEEP_DURATION, get_next_activity_time() - simgrid::s4u::Engine::get_clock());
    simgrid::s4u::this_actor::sleep_for(sleep_duration);
  }
  XBT_INFO("shutting down");
  simgrid::s4u::Actor::kill_all();
}

int BaseNode::get_id()
{
  return my_id;
}
