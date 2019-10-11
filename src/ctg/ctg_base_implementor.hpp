#ifndef CTG_BASE_IMPLEMENTOR_HPP
#define CTG_BASE_IMPLEMENTOR_HPP

#include "../client/node.hpp"
#include "../trace/trace_item.hpp"

class CTG_BaseImplementor
{
public:
  virtual TraceItem get_next_activity_item(Node *node) = 0;
};

#endif /* CTG_BASE_IMPLEMENTOR_HPP */
