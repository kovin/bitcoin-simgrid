#ifndef BASE_NODE_HPP
#define BASE_NODE_HPP

#include "simgrid/s4u.hpp"
#include "bitcoin_simgrid.hpp"
#include "magic_constants.hpp"
#include "message.hpp"
#include "aux_functions.hpp"
#include <cstdlib>
#include <set>
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

class BaseNode
{
public:
  void operator()()
  {
    XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);
    while (simgrid::s4u::Engine::getClock() < SIMULATION_DURATION) {
      generate_activity();
      process_messages();
      send_messages();
      simgrid::s4u::this_actor::sleep_for(SLEEP_DURATION);
    }
    XBT_DEBUG("shutting down");
    simgrid::s4u::Actor::killAll();
  }
protected:
  int my_id;
  std::vector<int> my_peers;
  json node_data;

  virtual void init_from_args(std::vector<std::string> args)
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

  /*
  * Returns the next expected arrival of an activity event following a poisson distribution where:
  *   probability: chance of an event by this actor in the next second
  *   timespan: is the amount of seconds, or time frame, in which we are interested. For example a day duration for the events of transactions per day
  *   events_per_timespan: is the amount of event that we expect to occur during the considered timespan. For example 10K transactions per day
  */
  double get_next_activity_time(double probability, int timespan, int events_per_timespan)
  {
    return simgrid::s4u::Engine::getClock() + (-log(1 - frand()) / probability) * timespan / events_per_timespan;
  }
  virtual std::string get_node_data_filename(int id) = 0;
  virtual void generate_activity() = 0;
  virtual void process_messages() = 0;
  virtual void send_messages() = 0;

private:
  virtual void do_set_next_activity_time() = 0;
};

#endif /* BASE_NODE_HPP */
