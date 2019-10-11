#include "base_node.hpp"
#include "../bitcoin_simgrid.hpp"

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
  nodes_knowing_block = {{0, NODES_COUNT}};
}

void BaseNode::operator()()
{
  next_time_for_global_activity = simgrid::s4u::Engine::get_clock();
  while (simgrid::s4u::Engine::get_clock() < SIMULATION_DURATION) {
    generate_activity();
    bool has_pending_work = handle_messages();
    if (!has_pending_work) {
      double next_activity_time = get_next_activity_time();
      double sleep_until_next_activity = next_activity_time - simgrid::s4u::Engine::get_clock();
      double sleep_duration = std::min(SLEEP_DURATION, sleep_until_next_activity);
      if (SKIP_TIME_WHEN_POSSIBLE) {
        sleep_duration = get_nex_sleep_time_with_perf_improvements(next_activity_time, sleep_duration);
      }
      simgrid::s4u::this_actor::sleep_for(sleep_duration);
    }
    if (signalHandler.gotExitSignal()) {
      LOG("FORCED shut down. real simulation time: %ld seconds", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - START_TIME).count());
      exit(111);
    }
  }
  LOG("shut down. real simulation time: %ld seconds", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - START_TIME).count());
  simgrid::s4u::Actor::kill_all();
}

double BaseNode::get_nex_sleep_time_with_perf_improvements(double next_activity_time, double sleep_duration)
{
  if (perf_improv_stage == PERF_STAGE_INIT) {
    if (sent_messages == received_messages) {
      perf_improv_stage = PERF_STAGE_MESSAGES_OK;
    }
  }
  if (perf_improv_stage == PERF_STAGE_MESSAGES_OK) {
    if (sent_messages != received_messages) {
      perf_improv_stage = PERF_STAGE_INIT;
      next_times_set = 0;
    } else {
      next_time_for_global_activity = (next_time_for_global_activity < simgrid::s4u::Engine::get_clock())
        ? next_activity_time
        : std::min(next_time_for_global_activity, next_activity_time);
      next_times_set++;
      if (next_times_set == NODES_COUNT) {
        perf_improv_stage = PERF_STAGE_NEXT_ACTIVITY_OK;
      }
    }
  }
  bool long_sleep_completed = long_sleep_completed_for_node_id.find(my_id) != long_sleep_completed_for_node_id.end();
  if ((perf_improv_stage == PERF_STAGE_NEXT_ACTIVITY_OK) && !long_sleep_completed) {
    long_sleep_completed_for_node_id.insert(my_id);
    sleep_duration = next_time_for_global_activity - simgrid::s4u::Engine::get_clock();
    next_times_set--;
    if (next_times_set == 0) {
      perf_improv_stage = PERF_STAGE_INIT;
      long_sleep_completed_for_node_id = {};
    }
  }
  return sleep_duration;
}

int BaseNode::get_id()
{
  return my_id;
}
