#include "magic_constants.hpp"
#include "bitcoin_simgrid.hpp"
#include "node.hpp"
#include "miner.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(bitcoin_simgrid, "bitcoing-simgrid logs");

// This is the directory where the nodes should go to look for their bootstrapping data
std::string deployment_directory;

// This will be the single instance in charge of centralizing the generation of transactions
CTG* ctg;

// By default we are goind to simulate an hour, but this can be changes using the --simulation-duration argument
unsigned int SIMULATION_DURATION = 3600;

void parse_and_validate_args(int argc, char *argv[]) {
  xbt_assert(argc <= 6, "Usage: %s platform_file deployment_directory [-debug] [--simulation-duration <seconds>]", argv[0]);
  if (argc > 3) {
    for (int i = 3; i < argc; ++i) {
      if (std::string(argv[i]) == "-debug") {
        xbt_log_control_set("bitcoin_simgrid.fmt:%10h:%e%m%n bitcoin_simgrid.thres:debug");
      } else if (std::string(argv[i]) == "--simulation-duration") {
        xbt_assert(argc > i, "Missing argument for --simulation-duration");
        ++i;
        SIMULATION_DURATION = std::stoi(argv[i]);
      } else {
        xbt_assert(false, "Unknown argument '%s'", argv[i]);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  simgrid::s4u::Engine e(&argc, argv);
  parse_and_validate_args(argc, argv);
  srand(1);// Use a constant seed to have deterministic runs on our simulations
  CTG::EXTENSION_ID = simgrid::s4u::Actor::extension_create<CTG>();
  e.register_actor<Node>("node");
  e.register_actor<Miner>("miner");
  e.load_platform(argv[1]);
  deployment_directory = std::string(argv[2]);
  // This will be the single instance in charge of centralizing the generation of transactions
  ctg = new CTG();
  std::string deployment_file = deployment_directory + std::string("/deployment.xml");
  e.load_deployment(deployment_file.c_str());
  e.run();
  return 0;
}
