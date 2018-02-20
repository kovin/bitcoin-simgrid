#include "aux-functions.hpp"

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
