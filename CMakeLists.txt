cmake_minimum_required(VERSION 3.5)

project(bitcoin-simgrid)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(SimGrid REQUIRED)
include_directories(${SimGrid_INCLUDE_DIR})

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 OLD)
endif(COMMAND cmake_policy)

add_executable (
    bitcoin-simgrid
    src/bitcoin_simgrid.cpp
    src/signal_handler.cpp
    src/aux_functions.cpp
    src/client/base_node.cpp
    src/client/node.cpp
    src/client/miner.cpp
    src/client/shared_data.cpp
    src/client/validator_timer.cpp
    src/ctg/ctg.cpp
    src/ctg/ctg_model_implementor.cpp
    src/ctg/ctg_trace_implementor.cpp
    src/trace/trace_item.cpp
    src/trace/trace_item_miner.cpp
)
target_link_libraries(bitcoin-simgrid simgrid)
set_target_properties(bitcoin-simgrid PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
foreach (file node miner aux-functions)
  set(examples_src ${examples_src} ${CMAKE_CURRENT_SOURCE_DIR}/src/${file}.cpp)
endforeach()

include_directories("${SimGrid_INCLUDE_DIR}" SYSTEM)
target_link_libraries(bitcoin-simgrid ${SimGrid_LIBRARY})
