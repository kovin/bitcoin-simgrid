#ifndef MONITOR_HPP
#define MONITOR_HPP

#include "node.hpp"

class Monitor
{
public:
  explicit Monitor(std::vector<std::string> args)
  {
  }
  void operator()();
};

#endif /* MONITOR_HPP */
