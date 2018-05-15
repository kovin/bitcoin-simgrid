#ifndef CTG_BASE_IMPLEMENTOR_HPP
#define CTG_BASE_IMPLEMENTOR_HPP

#include "node.hpp"

class CTG_BaseImplementor
{
public:
  virtual double get_next_activity_time(Node *node) = 0;
};

#endif /* CTG_BASE_IMPLEMENTOR_HPP */
