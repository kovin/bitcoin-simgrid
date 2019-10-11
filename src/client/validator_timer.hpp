#ifndef VALIDATOR_TIMER_HPP
#define VALIDATOR_TIMER_HPP

#include "../message.hpp"

/*
* This class is in charge of computing the needed time to simulate the validation of block and transactions.
* The validation time is relative to the size of the block or transactions:
* - the block validation follows a cuadratic increase in time relative to the size
* - the transaction validation follows an increase proportional to the transactions size
*/
class ValidatorTimer
{
public:
  double get_flops_to_process_block(Block block);
  double get_flops_to_process_transactions(std::map<long, Transaction> txs_to_validate);

private:
  double get_flops_to_process_transaction(Transaction tx);
};

#endif /* VALIDATOR_TIMER_HPP */
