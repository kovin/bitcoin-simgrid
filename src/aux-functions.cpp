#include "aux-functions.hpp"

long lrand()
{
  if (sizeof(int) < sizeof(long)) {
      return (static_cast<long>(rand()) << (sizeof(int) * 8)) | rand();
  } else {
    return rand();
  }
}
