#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "magic_constants.hpp"
#include "aux_functions.hpp"
#include <set>

typedef enum
{
  MESSAGE_BLOCK,
  MESSAGE_TX,
  MESSAGE_TXS,
  MESSAGE_INV,
  MESSAGE_GETDATA,
} e_message_type;

typedef enum
{
  INV_TX,
  INV_BLOCK
} e_inv_type;

class Message
{
public:
  Message(long size) : size(size)
  {
    id = lrand();
  }

  Message(): size(0)
  {
    id = lrand();
  }

  long get_id() const
  {
    return id;
  }

  long get_size() const
  {
    return size;
  }

  virtual e_message_type get_type() const = 0;

  ~Message() = default;
protected:
  long size;
private:
  long id;
};

class Transaction : public Message
{
public:
  Transaction() : Message(-1) {}

  Transaction(long size, long fee_per_byte, double confirmed) : Message(size), fee_per_byte(fee_per_byte), confirmed(confirmed) { };

  e_message_type get_type() const
  {
    return MESSAGE_TX;
  }

  long get_fee_per_byte() const
  {
    return fee_per_byte;
  }

  double get_confirmed() const
  {
    return confirmed;
  }
private:
  long fee_per_byte;
  double confirmed;
};

class Block : public Message
{
public:
  Block() : Message(-1), accumulated_difficulty(0) {}

  Block(int height, double time, long parent_id, unsigned long long network_difficulty, unsigned long long accumulated_difficulty, std::vector<Transaction> txs, int miner_id = 0)
  : Message(), height(height), time(time), parent_id(parent_id), network_difficulty(network_difficulty), accumulated_difficulty(accumulated_difficulty), transactions(txs), miner_id(miner_id)
  {
    for (auto tx : txs) {
      size += tx.get_size();
      transactions_map.insert(std::make_pair(tx.get_id(), tx));
    }
  };

  e_message_type get_type() const
  {
    return MESSAGE_BLOCK;
  }

  int get_height() const
  {
    return height;
  }

  long get_parent_id() const
  {
    return parent_id;
  }

  int get_miner_id() const
  {
    return miner_id;
  }

  std::vector<Transaction> get_transactions() const
  {
    return transactions;
  }

  std::map<long, Transaction> get_transactions_map() const
  {
    return transactions_map;
  }

  unsigned long long get_network_difficulty() const
  {
    return network_difficulty;
  }

  unsigned long long get_accumulated_difficulty() const
  {
    return accumulated_difficulty;
  }

  double get_time() const
  {
    return time;
  }
private:
  int height;
  long parent_id;
  std::vector<Transaction> transactions;
  std::map<long, Transaction> transactions_map;
  unsigned long long network_difficulty;
  unsigned long long accumulated_difficulty;
  double time;
  int miner_id;
};

class Transactions : public Message
{
public:
  Transactions(std::map<long, Transaction> txs) : Message(), transactions_map(txs) { };

  e_message_type get_type() const
  {
    return MESSAGE_TXS;
  }

  std::map<long, Transaction> get_transactions_map() const
  {
    return transactions_map;
  }
private:
  std::map<long, Transaction> transactions_map;
};

class Inv : public Message
{
public:
  Inv(std::map<long, e_inv_type> objects) : Message(BASE_MSG_SIZE), objects(objects) { };

  e_message_type get_type() const
  {
    return MESSAGE_INV;
  }

  std::map<long, e_inv_type> get_objects()
  {
    return objects;
  }
private:
  std::map<long, e_inv_type> objects;
};

class GetData : public Message
{
public:
  GetData(std::set<long> objects) : Message(BASE_MSG_SIZE), objects(objects) { };

  e_message_type get_type() const
  {
    return MESSAGE_GETDATA;
  }

  std::set<long> get_objects()
  {
    return objects;
  }
private:
  std::set<long> objects;
};

#endif /* MESSAGE_HPP */
