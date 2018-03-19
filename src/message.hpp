#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "aux_functions.hpp"
#include <set>

const long BLOCK_DIFFICULTY_THRESHOLD = 10000;

typedef enum {
  MESSAGE_BLOCK,
  MESSAGE_TRANSACTION,
  UNCONFIRMED_TRANSACTIONS
} e_message_type;

class Message
{
public:
  long id;
  int peer_id;
  long size;
  Message(int peer_id, long size) : peer_id(peer_id), size(size)
  {
    id = lrand();
  };
  Message(int peer_id): peer_id(peer_id), size(0)
  {
    id = lrand();
  };
  virtual e_message_type get_type() = 0;
  virtual std::string get_type_name() = 0;
  ~Message() = default;
};

class Transaction : public Message
{
  public:
    std::map<long, std::set<long>> inputs;// Map of referenced unspent outpoints to be used as inputs of this tx
    std::vector<long> outputs;// list of outputs created by this tx
    Transaction (int peer_id, long size) : Message(peer_id, size) { };

    e_message_type get_type()
    {
      return MESSAGE_TRANSACTION;
    }

    std::string get_type_name() {
      return "transaction";
    }
};

class Block : public Message
{
  public:
    long parent_id;
    std::map<long, Transaction> transactions;
    long difficulty;
    Block (int peer_id, long parent_id, std::map<long, Transaction> transactions) : Message(peer_id), parent_id(parent_id), transactions(transactions) {
        size += 1000000;
        for (auto const& idAndTransaction : transactions)
        {
          size += idAndTransaction.second.size;
        }
        difficulty = lrand(BLOCK_DIFFICULTY_THRESHOLD);
    };

    e_message_type get_type()
    {
      return MESSAGE_BLOCK;
    }

    std::string get_type_name() {
      return "block";
    }
};

class UnconfirmedTransactions : public Message
{
  public:
    std::map<long, Transaction> unconfirmed_transactions;
    UnconfirmedTransactions(int peer_id, std::map<long, Transaction> unconfirmed_transactions) : Message(peer_id), unconfirmed_transactions(unconfirmed_transactions) { };

    e_message_type get_type()
    {
      return UNCONFIRMED_TRANSACTIONS;
    }

    std::string get_type_name() {
      return "unconfirmed_transactions";
    }
};

#endif /* MESSAGE_HPP */
