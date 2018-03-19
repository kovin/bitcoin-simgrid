#include "bitcoin_simgrid.hpp"
#include "monitor.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

void Monitor::operator()()
{
  XBT_DEBUG("Time\t\tTotal network bytes produced");
  long last_bytes_monitored = 0;
  while (Node::active_nodes > 0) {
    if (last_bytes_monitored == Node::network_bytes_produced) {
      simgrid::s4u::this_actor::sleep_for(1);
    } else {
      last_bytes_monitored = Node::network_bytes_produced;
      XBT_DEBUG(
        "%f\t%ld",
        simgrid::s4u::Engine::getClock(),
        Node::network_bytes_produced
      );
    }
  }
  XBT_DEBUG("messages produced: %d", Node::messages_produced);
  XBT_DEBUG("messages received: %d", Node::messages_received);
}
