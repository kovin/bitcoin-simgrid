#ifndef BITCOIN_SIMGRID_HPP
#define BITCOIN_SIMGRID_HPP
#include <string>
#include "ctg.hpp"

// This is the directory where the nodes should go to look for their bootstrapping data
extern std::string deployment_directory;

// This will be the single instance in charge of centralizing the generation of transactions
extern CTG* ctg;

#endif /* BITCOIN_SIMGRID_HPP */
