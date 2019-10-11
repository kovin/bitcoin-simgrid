#include "validator_timer.hpp"

double ValidatorTimer::get_flops_to_process_block(Block block)
{
  double flops_to_process_block = 0;
  for (auto const& transaction : block.get_transactions()) {
    flops_to_process_block += get_flops_to_process_transaction(transaction);
  }
  return flops_to_process_block;
}

double ValidatorTimer::get_flops_to_process_transactions(std::map<long, Transaction> txs_to_validate)
{
  double flops_to_process_transactions = 0;
  for (auto const& idAndTransaction : txs_to_validate) {
    flops_to_process_transactions += get_flops_to_process_transaction(idAndTransaction.second);
  }
  return flops_to_process_transactions;
}

double ValidatorTimer::get_flops_to_process_transaction(Transaction tx)
{
  // Coefficients for f(x) = c2*x^2 + c1*x + c0
  // where:
  // - x is the tx size in bytes
  // - f(x) the time required to process it in microseconds
  double c2 = 0.0012956;
  double c1 = -0.32167;
  double c0 = 562.97;
  long x = tx.get_size();
  double microseconds = c2 * x * x + c1 * x + c0;
  // A standard host can compute 1Gf per second, which is 1e9 flops.
  // So, considering the standard computing power, a microsecond is the time 1e3 flops
  // would take. Given that the previous functions returns the time in microseconds
  // we multiply by 1e3 to get the number of expected flops to simulate.
  // Note: superhosts have more computing power proportional to their connectivity.
  return 1e3 * microseconds;
}
