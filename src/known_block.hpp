#ifndef KNOWN_BLOCK_HPP
#define KNOWN_BLOCK_HPP

#include <set>

class KnownBlock
{
public:
  long parent_id;
  long agregated_difficulty;
  double time;
  std::set<long> txs_ids;
  KnownBlock(long parent_id, long agregated_difficulty, double time, std::set<long> txs_ids)
  : parent_id(parent_id), agregated_difficulty(agregated_difficulty), time(time), txs_ids(txs_ids)
  {}
};

#endif /* KNOWN_BLOCK_HPP */
