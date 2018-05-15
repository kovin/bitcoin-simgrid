#ifndef CTG_MODEL_IMPLEMENTOR_HPP
#define CTG_MODEL_IMPLEMENTOR_HPP

#include "ctg_base_implementor.hpp"

using json = nlohmann::json;

class CTG_ModelImplementor : public CTG_BaseImplementor
{
public:
  explicit CTG_ModelImplementor(json ctg_data);
  double get_next_activity_time(Node *node);

private:
    int nodes_count;
    int txs_per_day;
    std::vector<double> event_probability;
    void compute_exponential_distribution(json ctg_data);
    void compute_uniform_distribution(json ctg_data);
};

#endif /* CTG_MODEL_IMPLEMENTOR_HPP */
