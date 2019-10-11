#ifndef AUX_FUNCTIONS_HPP
#define AUX_FUNCTIONS_HPP

#include "simgrid/s4u.hpp"
#include "magic_constants.hpp"
#include <cstdlib>
#include <set>

long lrand(long limit = 0);
unsigned long long llrand(unsigned long long limit = 0);
double frand(double limit = 0);
  /*
  * Returns the next expected arrival of an activity event following a poisson distribution where:
  *   probability: chance of an event by this actor in the next second
  *   timespan: is the amount of seconds, or time frame, in which we are interested. For example a day duration for the events of transactions per day
  *   events_per_timespan: is the amount of event that we expect to occur during the considered timespan. For example 10K transactions per day
  */
double calc_next_activity_time(double basetime, double probability, int timespan, int events_per_timespan);

template<typename KeyType, typename Value, typename OtherValue>
std::map<KeyType, Value> DiffMaps(const std::map<KeyType, Value> & left, const std::map<KeyType, OtherValue> & right)
{
  std::map<KeyType, Value> result;
  typename std::map<KeyType, Value>::const_iterator il = left.begin();
  typename std::map<KeyType, OtherValue>::const_iterator ir = right.begin();
  while (il != left.end()) {
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
  while (il != left.end()) {
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
  while (il != left.end()) {
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
  while (il != left.end() || ir != right.end()) {
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
  while (il != left.end() || ir != right.end()) {
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

template<typename KeyType, typename Value, typename OtherValue>
std::map<KeyType, Value> IntersectMaps(const std::map<KeyType, Value> & left, const std::map<KeyType, OtherValue> & right)
{
  std::map<KeyType, Value> result;
  typename std::map<KeyType, Value>::const_iterator il = left.begin();
  typename std::map<KeyType, OtherValue>::const_iterator ir = right.begin();
  while (il != left.end() && ir != right.end()) {
    if (il->first == ir->first) {
      result.insert(*il);
      ++il;
      ++ir;
    } else if (il->first < ir->first) {
      ++il;
    } else {
      ++ir;
    }
  }
  return result;
}

template<typename KeyType, typename Value>
std::map<KeyType, Value> IntersectMaps(const std::map<KeyType, Value> & left, const std::set<KeyType> & right)
{
  std::map<KeyType, Value> result;
  typename std::map<KeyType, Value>::const_iterator il = left.begin();
  typename std::set<KeyType>::const_iterator ir = right.begin();
  while (il != left.end() && ir != right.end()) {
    if (il->first == *ir) {
      result.insert(*il);
      ++il;
      ++ir;
    } else if (il->first < *ir) {
      ++il;
    } else {
      ++ir;
    }
  }
  return result;
}

template<typename KeyType, typename Value, typename... Args>
std::map<KeyType, Value> IntersectMaps(const std::map<KeyType, Value> & left, const std::set<KeyType> & right, const std::set<Args> &... args)
{
  const int n = sizeof...(args);
  return (n > 0)
    ? IntersectMaps(IntersectMaps(left, right), args...)
    : IntersectMaps(left, right);
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

template<typename KeyType, typename Value>
std::set<KeyType> JoinSets(const std::set<KeyType> & left, const std::map<KeyType, Value> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::map<KeyType, Value>::const_iterator ir = right.begin();
  while (il != left.end() || ir != right.end()) {
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
std::set<KeyType> DiffSets(const std::set<KeyType> & left, const std::set<KeyType> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::set<KeyType>::const_iterator ir = right.begin();
  while (il != left.end()) {
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

template<typename KeyType, typename... Args>
std::set<KeyType> DiffSets(const std::set<KeyType> & left, const std::set<KeyType> & right, const std::set<Args> &... args)
{
  const int n = sizeof...(args);
  return (n > 0)
    ? DiffSets(DiffSets(left, right), args...)
    : DiffSets(left, right);
}

template<typename KeyType, typename Value>
std::set<KeyType> DiffSets(const std::set<KeyType> & left, const std::map<KeyType, Value> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::map<KeyType, Value>::const_iterator ir = right.begin();
  while (il != left.end()) {
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

template<typename KeyType>
std::set<KeyType> IntersectSets(const std::set<KeyType> & left, const std::set<KeyType> & right)
{
  std::set<KeyType> result;
  typename std::set<KeyType>::const_iterator il = left.begin();
  typename std::set<KeyType>::const_iterator ir = right.begin();
  while (il != left.end() && ir != right.end()) {
    if (*il == *ir) {
      result.insert(*il);
      ++il;
      ++ir;
    } else if (*il < *ir) {
      ++il;
    } else {
      ++ir;
    }
  }
  return result;
}

template<typename KeyType, typename Value>
void Erase(std::map<KeyType, Value> & map_to_modify, KeyType key_to_remove)
{
  typename std::map<KeyType, Value>::const_iterator it = map_to_modify.find(key_to_remove);
  if (it != map_to_modify.end()) {
    map_to_modify.erase(it);
  }
}

template<typename KeyType>
void Erase(std::set<KeyType> & set_to_modify, KeyType key_to_remove)
{
  typename std::set<KeyType>::const_iterator it = set_to_modify.find(key_to_remove);
  if (it != set_to_modify.end()) {
    set_to_modify.erase(it);
  }
}

#define LOG(...) \
      do {                     \
        XBT_INFO(__VA_ARGS__); \
      }  while (0)

#define DEBUG(...) \
      do {                       \
        if (ENABLE_DEBUG) {      \
          XBT_INFO(__VA_ARGS__); \
        }                        \
      }  while (0)

#endif /* AUX_FUNCTIONS_HPP */
