#include "aux_functions.hpp"
#include "magic_constants.hpp"
#include <random>

long lrand(long limit)
{
  long result;
  if (sizeof(int) < sizeof(long)) {
    result = (static_cast<long>(rand()) << (sizeof(int) * 8)) | rand();
  } else {
    result = rand();
  }
  return limit ? result % limit : result;
}

long long llrand(long long limit)
{
  long long result;
  if (sizeof(long) < sizeof(long long)) {
    result = (static_cast<long long>(lrand()) << ((sizeof(long long) - sizeof(long)) * 8)) | lrand();
  } else {
    result = lrand();
  }
  return limit ? result % limit : result;
}

std::default_random_engine re;
double frand(double limit)
{
  std::uniform_real_distribution<double> unif(0, limit ? limit : 1);
 return unif(re);
}

double calc_next_activity_time(double probability, int timespan, int events_per_timespan)
{
  if (events_per_timespan > 0) {
    return simgrid::s4u::Engine::getClock() + (-log(1 - frand()) / probability) * timespan / events_per_timespan;
  } else {
    return SIMULATION_DURATION;
  }
}
