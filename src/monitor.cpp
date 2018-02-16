#include "bitcoin-simgrid.hpp"
#include "monitor.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

void Monitor::operator()()
{
  XBT_INFO("\nTime\t\tTotal network bytes produced");
  long last_bytes_monitored = Node::network_bytes_produced;
  while (Node::active_nodes > 0) {
    if (last_bytes_monitored != Node::network_bytes_produced) {
        last_bytes_monitored = Node::network_bytes_produced;
        XBT_INFO(
          "%f\t%ld\n",
          simgrid::s4u::Engine::getClock(),
          Node::network_bytes_produced
        );
    }
    simgrid::s4u::this_actor::sleep_for(1);
  }
  XBT_INFO("blockchain data: %d", blockchain_data);
}