#ifndef BASE_NODE_HPP
#define BASE_NODE_HPP

#include "simgrid/s4u.hpp"
#include "../magic_constants.hpp"
#include "../message.hpp"
#include "../aux_functions.hpp"
#include "../json.hpp"
#include <cstdlib>
#include <set>
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>
#include <fstream>
#include "shared_data.hpp"

using json = nlohmann::json;

/*
* This is the common class for both nodes and miners, in its process() method it will
* run the loop where we:
* - generate activity (if needed)
* - handle message (receive and send message from/to the peers of this node)
* - sleep for SLEEP_DURATION or until the next activity, whichever comes first
*/
class BaseNode
{
public:
  void operator()();
  int get_id();
  virtual double get_next_activity_time() = 0;
protected:
  int my_id;
  std::vector<int> my_peers;
  json node_data;

  virtual void init_from_args(std::vector<std::string> args);
  virtual std::string get_node_data_filename(int id) = 0;
  virtual void generate_activity() = 0;
  virtual bool handle_messages() = 0;
private:
  double get_nex_sleep_time_with_perf_improvements(double next_activity_time, double sleep_duration);
};

#endif /* BASE_NODE_HPP */
