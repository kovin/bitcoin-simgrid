#include <random>
#include "ctg.hpp"
#include "bitcoin_simgrid.hpp"

CTG::CTG()
{
  std::string ctg_data_filename = deployment_directory + "ctg_data";
  std::ifstream ctg_data_stream(ctg_data_filename);
  json ctg_data;
  ctg_data_stream >> ctg_data;
  if (ctg_data["mode"].get<std::string>() == "model") {
    implementor = new CTG_ModelImplementor(ctg_data);
  }
}
