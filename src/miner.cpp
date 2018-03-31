#include "miner.hpp"

XBT_LOG_EXTERNAL_DEFAULT_CATEGORY(bitcoin_simgrid);

Miner::Miner(std::vector<std::string> args): Node::Node()
{
  init_from_args(args);
  simgrid::s4u::this_actor::onExit((int_f_pvoid_pvoid_t) on_exit, NULL);
}

void Miner::init_from_args(std::vector<std::string> args)
{
  Node::init_from_args(args);
  difficulty = node_data["difficulty"].get<long long>();
  xbt_assert(difficulty > 0, "Network difficulty must be greater than 0, got %lld", difficulty);
  hashrate = node_data["hashrate"].get<long long>();
  xbt_assert(hashrate >= 0, "Miner hashrate can't be negative, got %lld", hashrate);
  do_set_next_activity_time();
}

void Miner::do_set_next_activity_time()
{
  int timespan = 600; // 10 minutes
  double shares = (hashrate / pow(2, 32)) * timespan ;
  double event_probability = 1 - pow(1 - 1.0 / difficulty, shares);
  next_activity_time = get_next_activity_time(event_probability, timespan, 1);
}

void Miner::generate_activity()
{
  Node::generate_activity();
  if (next_activity_time > simgrid::s4u::Engine::getClock()) {
    return;
  }
  do_set_next_activity_time();
  // TODO: deberia pasarle la dificultad con la que estoy operando a Block. El bloque deberia crearse con la fecha actual tambien
  Block* block = new Block(my_id, blockchain_top, mempool);
  blocks_to_broadcast.insert(std::make_pair(block->id, *block));
  // TODO: deberia hacer el set del nuevo top de la blockchain en un metodo de la clase Node, tipo set_new_tip(Block block)
  // , en ese mismo metodo debería recalcular la dificultad actual si el bloque es multiplo de 2016
  long previous_difficulty = known_blocks[blockchain_top];
  known_blocks[block->id] = block->difficulty + previous_difficulty;
  blockchain_top = block->id;
  mempool = std::map<long, Transaction>();
}
