#ifndef CTG_HPP
#define CTG_HPP

#include "node.hpp"
#include "ctg_base_implementor.hpp"

using json = nlohmann::json;

class CTG
{
public:
  explicit CTG();
  ~CTG();

  double get_next_activity_time(Node *node) {
    return implementor->get_next_activity_time(node);
  }

private:
    CTG_BaseImplementor* implementor;
};

#endif /* CTG_HPP */
