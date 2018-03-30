#ifndef AUX_FUNCTIONS_HPP
#define AUX_FUNCTIONS_HPP

#include "simgrid/s4u.hpp"
#include <cstdlib>

long lrand(long limit = 0);
double frand();

template<typename KeyType, typename Value>
std::map<KeyType, Value> DiffMaps(const std::map<KeyType, Value> & left, const std::map<KeyType, Value> & right)
{
  std::map<KeyType, Value> result;
  typename std::map<KeyType, Value>::const_iterator il = left.begin();
  typename std::map<KeyType, Value>::const_iterator ir = right.begin();
  while (il != left.end())
  {
    if (ir == right.end() || il->first < ir->first) {
      result.insert(std::make_pair(il->first, il->second));
      ++il;
    } else if (ir != right.end()) {
      if (il->first == ir->first) {
        ++il;
      }
      ++ir;
    }
  }
  return result;
}

template<typename KeyType, typename Value>
std::map<KeyType, Value> JoinMaps(const std::map<KeyType, Value> & left, const std::map<KeyType, Value> & right)
{
  std::map<KeyType, Value> result;
  typename std::map<KeyType, Value>::const_iterator il = left.begin();
  typename std::map<KeyType, Value>::const_iterator ir = right.begin();
  while (il != left.end() || ir != right.end())
  {
    if (il != left.end()) {
      result.insert(std::make_pair(il->first, il->second));
      ++il;
    }
    if (ir != right.end()) {
      result.insert(std::make_pair(ir->first, ir->second));
      ++ir;
    }
  }
  return result;
}

template<typename KeyType, typename Value>
std::map<KeyType, Value> InsersectMaps(const std::map<KeyType, Value> & left, const std::map<KeyType, Value> & right)
{
  std::map<KeyType, Value> result;
  typename std::map<KeyType, Value>::const_iterator il = left.begin();
  typename std::map<KeyType, Value>::const_iterator ir = right.begin();
  while (il != left.end() && ir != right.end())
  {
    if (il->first == ir->first) {
      result.insert(std::make_pair(il->first, il->second));
    } else if (il->first < ir->first) {
      ++il;
    } else {
      ++ir;
    }
  }
  return result;
}

#endif /* AUX_FUNCTIONS_HPP */
