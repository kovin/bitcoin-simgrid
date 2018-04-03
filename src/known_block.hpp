#ifndef KNOWN_BLOCK_HPP
#define KNOWN_BLOCK_HPP

#include <set>

class KnownBlock
{
public:
  int height;
  int parent_height;
  long long agregated_difficulty;
  double time;
  std::set<long> txs_ids;
  KnownBlock(int height, int parent_height, long long agregated_difficulty, double time, std::set<long> txs_ids)
  : height(height), parent_height(parent_height), agregated_difficulty(agregated_difficulty), time(time), txs_ids(txs_ids)
  {}
};

#endif /* KNOWN_BLOCK_HPP */
