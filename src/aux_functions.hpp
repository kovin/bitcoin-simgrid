#ifndef AUX_FUNCTIONS_HPP
#define AUX_FUNCTIONS_HPP

#include "simgrid/s4u.hpp"
#include <cstdlib>
#include <set>

long lrand(long limit = 0);
long long llrand(long long limit = 0);
double frand(double limit = 0);

template<typename KeyType, typename Value>
std::set<KeyType> JustKeys(const std::map<KeyType, Value> & some_map)
{
  std::set<KeyType> result;
  typename std::map<KeyType, Value>::const_iterator it = some_map.begin();
  while (it != some_map.end())
  {
    result.insert(it->first);
    ++it;
  }
  return result;
}

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
std::set<KeyType> DiffMaps(const std::set<KeyType> & left, const std::map<KeyType, Value> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::map<KeyType, Value>::const_iterator ir = right.begin();
  while (il != left.end())
  {
    if (ir == right.end() || *il < ir->first) {
      result.insert(*il);
      ++il;
    } else if (ir != right.end()) {
      if (*il == ir->first) {
        ++il;
      }
      ++ir;
    }
  }
  return result;
}

template<typename KeyType, typename Value>
std::map<KeyType, Value> DiffMaps(const std::map<KeyType, Value> & left, const std::set<KeyType> & right)
{
  std::map<KeyType, Value> result;
  typename std::map<KeyType, Value>::const_iterator il = left.begin();
  typename std::set<KeyType>::const_iterator ir = right.begin();
  while (il != left.end())
  {
    if (ir == right.end() || il->first < *ir) {
      result.insert(std::make_pair(il->first, il->second));
      ++il;
    } else if (ir != right.end()) {
      if (il->first == *ir) {
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
std::set<KeyType> JoinMaps(const std::set<KeyType> & left, const std::map<KeyType, Value> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::map<KeyType, Value>::const_iterator ir = right.begin();
  while (il != left.end() || ir != right.end())
  {
    if (il != left.end()) {
      result.insert(*il);
      ++il;
    }
    if (ir != right.end()) {
      result.insert(ir->first);
      ++ir;
    }
  }
  return result;
}

template<typename KeyType>
std::set<KeyType> JoinSets(const std::set<KeyType> & left, const std::set<KeyType> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::set<KeyType>::const_iterator ir = right.begin();
  while (il != left.end() || ir != right.end())
  {
    if (il != left.end()) {
      result.insert(*il);
      ++il;
    }
    if (ir != right.end()) {
      result.insert(*ir);
      ++ir;
    }
  }
  return result;
}

template<typename KeyType>
std::set<KeyType> DiffSets(const std::set<KeyType> & left, const std::set<KeyType> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::set<KeyType>::const_iterator ir = right.begin();
  while (il != left.end())
  {
    if (ir == right.end() || *il < *ir) {
      result.insert(*il);
      ++il;
    } else if (ir != right.end()) {
      if (*il == *ir) {
        ++il;
      }
      ++ir;
    }
  }
  return result;
}

template<typename KeyType>
std::set<KeyType> InsersectSets(const std::set<KeyType> & left, const std::set<KeyType> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::set<KeyType>::const_iterator ir = right.begin();
  while (il != left.end() && ir != right.end())
  {
    if (*il == *ir) {
      result.insert(*il);
    } else if (*il < *ir) {
      ++il;
    } else {
      ++ir;
    }
  }
  return result;
}

#endif /* AUX_FUNCTIONS_HPP */
