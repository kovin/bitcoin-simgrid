#include "bitcoin-simgrid.hpp"
#include "node.hpp"
#include "miner.hpp"
#include "monitor.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(bitcoin_simgrid, "Messages specific for this msg example");


int main(int argc, char *argv[])
{
  simgrid::s4u::Engine e(&argc, argv);

  xbt_assert((argc - 1) == 3, "Usage: %s platform_file deployment_file blockchain_data", argv[0]);
  srand(1);
  e.registerFunction<Node>("node");
  e.registerFunction<Miner>("miner");
  e.registerFunction<Monitor>("monitor");

  e.loadPlatform(argv[1]);
  e.loadDeployment(argv[2]);
  e.run();
  return 0;
}
