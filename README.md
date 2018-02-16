# bitcoin-simgrid

## Pre requisites
$ sudo apt-get install libboost-all-dev cmake valgrind
$ git clone git://scm.gforge.inria.fr/simgrid/simgrid.git simgrid
$ cd simgrid
simgrid$ cmake -DCMAKE_INSTALL_PREFIX=/opt/simgrid .
simgrid$ make
simgrid$ sudo make install

## Build
bitcoin-simgrid$ cmake .
bitcoin-simgrid$ make bitcoin-simgrid

## Run
bitcoin-simgrid$ bin/bitcoin-simgrid platform/exponential_100_nodes_.5_alpha.xml platform/small_deployment.xml platform/blockchain_data.json
