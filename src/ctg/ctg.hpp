#ifndef CTG_HPP
#define CTG_HPP

#include "../client/node.hpp"
#include "ctg_base_implementor.hpp"

using json = nlohmann::json;

/*
* CTG is the Central Transaction Generator which offers 2 ways to tell each
* node when it should create the next tx:
* 1) CTG_ModelImplementor provides a strategy where we produce n txs per hour which are distributed
* among either in a uniform way or following an exponential distribution
* 2) CTG_TraceImplementor reads txs from a real blockchain trace and distributes those txs among nodes
* following a uniform distribution
*/
class CTG
{
public:
  explicit CTG();
  ~CTG();

  TraceItem get_next_activity_item(Node *node) {
    return implementor->get_next_activity_item(node);
  }

private:
  CTG_BaseImplementor* implementor;
};

#endif /* CTG_HPP */
