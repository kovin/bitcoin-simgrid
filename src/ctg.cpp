#include "ctg.hpp"
#include "bitcoin_simgrid.hpp"
#include "ctg_model_implementor.hpp"
#include "ctg_trace_implementor.hpp"

CTG::CTG()
{
  std::string ctg_data_filename = deployment_directory + "ctg_data";
  std::ifstream ctg_data_stream(ctg_data_filename);
  json ctg_data;
  ctg_data_stream >> ctg_data;
  std::string mode = ctg_data["mode"].get<std::string>();
  xbt_assert(mode == "model" || mode == "trace", "CTG model should be either 'model' or 'trace'");
  if (mode == "model") {
    implementor = new CTG_ModelImplementor(ctg_data);
  } else {
    implementor = new CTG_TraceImplementor(ctg_data);
  }
}

CTG::~CTG()
{
  delete implementor;
}
