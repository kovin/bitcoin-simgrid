#include "magic_constants.hpp"
#include "bitcoin_simgrid.hpp"
#include "client/node.hpp"
#include "client/miner.hpp"
#include "xbt/config.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(bitcoin_simgrid, "bitcoing-simgrid logs");

// This is the directory where the nodes should go to look for their bootstrapping data
std::string deployment_directory;

// This will be the single instance in charge of centralizing the generation of transactions
CTG* ctg;

// Set-up signal handler to detect forced exits
SignalHandler signalHandler;

// Provide START_TIME as a way to log the real time a simulation took to complete
std::chrono::steady_clock::time_point START_TIME = std::chrono::steady_clock::now();;

// By default we are goind to simulate an hour, but this can be changed using the --simulation-duration argument
unsigned int SIMULATION_DURATION = 3600;

// By default we are goind to use 10 minutes as the target time for a block, but this can be changed using the --target-time argument
unsigned int INTERVAL_BETWEEN_BLOCKS_IN_SECONDS = 600;

// Json encoded hashrate is limited and can't represent legitimate high values.
// So the tool accepts lower json encoded hashrate values that can then be up-scaled using this constant
unsigned int HASHRATE_SCALE = 1;

// How long we'll sleep before checking for new messages when there's no more work to do
double SLEEP_DURATION = .1;

// Allow to set random generator seed in order to reproduce simulations in a deterministic way
unsigned int SEED = 1;

// Set default random number generator
std::default_random_engine re;

// In the main() function we'll init this value with the sum of nodes (normal and miners) that are part of the current simulation
unsigned int NODES_COUNT;

// If true, then we will avoid the loop events of each node when we know there are no more messages to receive/send
// until the next global activity in the network
bool SKIP_TIME_WHEN_POSSIBLE = false;

// If true, more information about transactions and blocks will be included in the produced log
bool ENABLE_DEBUG = false;

std::string get_usage() {
  return "Usage: %s platform_file deployment_directory \n"
    "\t[--simulation-duration <seconds>]\n"
    "\t[--target-time <seconds>]\n"
    "\t[--sleep-duration <milliseconds>]\n"
    "\t[--seed <number>]\n"
    "\t[--custom-log]\n"
    "\t[--hashrate-scale <number>]\n"
    "\t[--skip-time-when-possible]\n"
    "\t[--debug]";
}

void print_usage()
{
  std::cout << get_usage() << std::endl;
  exit(0);
}

bool usingCustomLog = false;
void parse_and_validate_args(int argc, char *argv[])
{
  xbt_assert(
    argc <= 13 && argc >= 3,
    get_usage().c_str(),
    argv[0]
  );
  if (argc > 3) {
    for (int i = 3; i < argc; ++i) {
      if (std::string(argv[i]) == "--simulation-duration") {
        xbt_assert(argc > (i + 1), "Missing argument for --simulation-duration");
        ++i;
        SIMULATION_DURATION = std::stoi(argv[i]);
      } else if (std::string(argv[i]) == "--target-time") {
        xbt_assert(argc > (i + 1), "Missing argument for --target-time");
        ++i;
        INTERVAL_BETWEEN_BLOCKS_IN_SECONDS = std::stoi(argv[i]);
      } else if (std::string(argv[i]) == "--sleep-duration") {
        xbt_assert(argc > (i + 1), "Missing argument for --sleep-duration");
        ++i;
        SLEEP_DURATION = std::stod(argv[i]) / 1000;
      } else if (std::string(argv[i]) == "--seed") {
        xbt_assert(argc > (i + 1), "Missing argument for --seed");
        ++i;
        SEED = std::stod(argv[i]);
      } else if (std::string(argv[i]) == "--hashrate-scale") {
        xbt_assert(argc > (i + 1), "Missing argument for --hashrate-scale");
        ++i;
        HASHRATE_SCALE = std::stoi(argv[i]);
      } else if (std::string(argv[i]) == "--custom-log") {
        usingCustomLog = true;
      } else if (std::string(argv[i]) == "--skip-time-when-possible") {
        SKIP_TIME_WHEN_POSSIBLE = true;
      } else if (std::string(argv[i]) == "--debug") {
        ENABLE_DEBUG = true;
      } else if (std::string(argv[i]) == "--help") {
        print_usage();
      } else {
        xbt_assert(false, "Unknown argument '%s'", argv[i]);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  parse_and_validate_args(argc, argv);
  simgrid::s4u::Engine e(&argc, argv);
  if (!usingCustomLog) {
      // Specify a nice output by default
      xbt_log_control_set("root.thres:CRITICAL bitcoin_simgrid.thres:INFO bitcoin_simgrid.fmt:%d%10h:%e%m%n");
  }
  // Specify a network model without latency assumptions that are an order of magnitude higher than need to be. See https://lists.gforge.inria.fr/pipermail/simgrid-user/2017-July/004322.html
  simgrid::config::set_parse("network/model:SMPI");
  srand(SEED);
  re.seed(SEED);
  e.register_actor<Node>("node");
  e.register_actor<Miner>("miner");
  e.load_platform(argv[1]);
  deployment_directory = std::string(argv[2]) + "/";
  // This will be the single instance in charge of centralizing the generation of transactions
  ctg = new CTG();
  std::string deployment_file = deployment_directory + std::string("/deployment.xml");
  e.load_deployment(deployment_file.c_str());
  NODES_COUNT = e.get_actor_count();
  // Register signal handler to handle kill signal
  signalHandler.setupSignalHandlers();
  e.run();
  return 0;
}
