# bitcoin-simgrid

## Prerequisites
```bash
$ sudo apt-get install libboost-all-dev cmake valgrind python-networkx git
$ git clone git://scm.gforge.inria.fr/simgrid/simgrid.git simgrid
$ cd simgrid
simgrid$ cmake -DCMAKE_INSTALL_PREFIX=/opt/simgrid .
simgrid$ make
simgrid$ sudo make install
```

## Build
```bash
bitcoin-simgrid$ cmake .
bitcoin-simgrid$ make bitcoin-simgrid
```

## Run
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/exponential_100_nodes_.5_alpha.xml platform/small_deployment.xml platform/blockchain_data.json
```

# Topology generation
## With Full routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_full_routing.xml --nodes=300 --edges=2 --routing=Full --seed=1
```
## With Dijkstra routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_dijkstra_routing.xml --nodes=300 --edges=2 --routing=Dijkstra --seed=1
```
## With DijkstraCache routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_dijkstracache_routing.xml --nodes=300 --edges=2 --routing=DijkstraCache --seed=1
```

# Deployment generation
```bash
bitcoin-simgrid$ utils/createDeploymentXml --file=300_nodes_deployment.xml --nodes=300
```
