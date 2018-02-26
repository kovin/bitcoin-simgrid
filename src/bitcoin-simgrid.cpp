#include "bitcoin-simgrid.hpp"
#include "node.hpp"
#include "miner.hpp"
#include "monitor.hpp"
#include <fstream>
#include "json.hpp"
using json = nlohmann::json;

XBT_LOG_NEW_DEFAULT_CATEGORY(bitcoin_simgrid, "Messages specific for this msg example");

//int blockchain_data = 10;

int main(int argc, char *argv[])
{
  simgrid::s4u::Engine e(&argc, argv);

  xbt_assert((argc - 1) == 3, "Usage: %s platform_file deployment_file blockchain_data", argv[0]);
  srand(1);
//blockchain_data = 1000;
  e.registerFunction<Node>("node");
  e.registerFunction<Miner>("miner");
  e.registerFunction<Monitor>("monitor");

  e.loadPlatform(argv[1]);
  e.loadDeployment(argv[2]);
  e.run();

/*json j2 = {
  {"pi", 3.141},
  {"happy", true},
  {"name", "Niels"},
  {"nothing", nullptr},
  {"answer", {
    {"everything", 42}
  }},
  {"list", {1, 0, 2}},
  {"object", {
    {"currency", "USD"},
    {"value", 42.99}
  }}
};

  std::ifstream i(argv[3]);
  xbt_assert(i.good(), "File %s doesn't exist or the program doesn't have permission to read it", argv[3]);

  json j;
  i >> j;

  // write prettified JSON to another file
  std::ofstream o("pretty.json");
  o << std::setw(4) << j << std::endl;

  XBT_INFO("something: %d\n", j2["answer"]["everything"].get<int>());
*/
  return 0;
}
