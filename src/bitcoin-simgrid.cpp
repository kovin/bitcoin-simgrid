#include "bitcoin-simgrid.hpp"
#include "node.hpp"
#include "miner.hpp"
#include "monitor.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(bitcoin_simgrid, "bitcoing-simgrid logs");

std::string deployment_directory;

int main(int argc, char *argv[])
{
  simgrid::s4u::Engine e(&argc, argv);
  xbt_assert((argc - 1) == 2, "Usage: %s platform_file deployment_directory", argv[0]);
  srand(1);
  e.registerFunction<Node>("node");
  e.registerFunction<Miner>("miner");
  e.registerFunction<Monitor>("monitor");
  e.loadPlatform(argv[1]);
  deployment_directory = std::string(argv[2]);
  std::string deployment_file = deployment_directory + std::string("/deployment.xml");
  e.loadDeployment(deployment_file.c_str());
  e.run();
  return 0;
}
