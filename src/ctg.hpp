#ifndef CTG_HPP
#define CTG_HPP

#include "node.hpp"

using json = nlohmann::json;

class CTG
{
public:
  static simgrid::xbt::Extension<simgrid::s4u::Actor, CTG> EXTENSION_ID;

  explicit CTG();
  double get_next_activity_time(Node *node);

private:
    double event_probability;
    int txs_per_day;
};

#endif /* CTG_HPP */
