#ifndef BASE_NODE_HPP
#define BASE_NODE_HPP
#define SLEEP_DURATION 1

#include "simgrid/s4u.hpp"
#include "message.hpp"
#include <cstdlib>
#include <set>
#include <iostream>
#include <string>
#include <sstream>

class BaseNode
{
protected:
  virtual void send_messages() = 0;
  virtual Message* get_message_to_send() = 0;
};

#endif /* BASE_NODE_HPP */
