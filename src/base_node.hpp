#ifndef BASE_NODE_HPP
#define BASE_NODE_HPP

#include "simgrid/s4u.hpp"
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
  int get_id();
  virtual double get_next_activity_time() = 0;
protected:
  int my_id;
  std::vector<int> my_peers;
  json node_data;

  virtual void init_from_args(std::vector<std::string> args);
  virtual std::string get_node_data_filename(int id) = 0;
  virtual void generate_activity() = 0;
  virtual void process_messages() = 0;
  virtual void send_messages() = 0;
};

#endif /* BASE_NODE_HPP */
