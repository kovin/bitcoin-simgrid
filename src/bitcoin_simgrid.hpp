#ifndef BITCOIN_SIMGRID_HPP
#define BITCOIN_SIMGRID_HPP
#include <string>
#include <chrono>
#include "ctg/ctg.hpp"
#include "signal_handler.hpp"

// This is the directory where the nodes should go to look for their bootstrapping data
extern std::string deployment_directory;

// This will be the single instance in charge of centralizing the generation of transactions
extern CTG* ctg;

// Set-up signal handler to detect forced exits
extern SignalHandler signalHandler;

// Provide START_TIME as a way to log the real time a simulation took to complete
extern std::chrono::steady_clock::time_point START_TIME;

#endif /* BITCOIN_SIMGRID_HPP */
