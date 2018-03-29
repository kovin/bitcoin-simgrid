#include "aux_functions.hpp"
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

std::default_random_engine re;
double frand()
{
  std::uniform_real_distribution<double> unif(0, 1);
 return unif(re);
}
