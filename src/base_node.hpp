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
  void operator()();
protected:
  int my_id;
  std::vector<int> my_peers;
  json node_data;

  virtual void init_from_args(std::vector<std::string> args);

  /*
  * Returns the next expected arrival of an activity event following a poisson distribution where:
  *   probability: chance of an event by this actor in the next second
  *   timespan: is the amount of seconds, or time frame, in which we are interested. For example a day duration for the events of transactions per day
  *   events_per_timespan: is the amount of event that we expect to occur during the considered timespan. For example 10K transactions per day
  */
  double calc_next_activity_time(double probability, int timespan, int events_per_timespan);
  virtual double get_next_activity_time() = 0;
  virtual std::string get_node_data_filename(int id) = 0;
  virtual void generate_activity() = 0;
  virtual void process_messages() = 0;
  virtual void send_messages() = 0;
};

#endif /* BASE_NODE_HPP */
