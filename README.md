# bitcoin-simgrid

## Prerequisites
```bash
$ sudo apt-get install doxygen libboost-all-dev cmake valgrind python-networkx git
$ git clone git://scm.gforge.inria.fr/simgrid/simgrid.git simgrid
$ cd simgrid
simgrid$ cmake -DCMAKE_INSTALL_PREFIX=/opt/simgrid .
simgrid$ make
simgrid$ sudo make install
```

## Clone
```bash
$ git clone git@github.com:kovin/bitcoin-simgrid.git
$ cd bitcoin-simgrid
```

## Build
```bash
bitcoin-simgrid$ cmake .
bitcoin-simgrid$ make
```

## Run
### Without full logging
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default_platform.xml platform/default_deployment.xml platform/default_blockchain_data.json --log="root.fmt:%m"

```
### With debug logging
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default_platform.xml platform/default_deployment.xml platform/default_blockchain_data.json --log="root.fmt:%m%n" --log=root.threshold:debug

```

## Topology generation
### With Full routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_full_routing.xml --hosts_count=300 --edges=2 --routing=Full --seed=1
```
### With Dijkstra routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_dijkstra_routing.xml --hosts_count=300 --edges=2 --routing=Dijkstra --seed=1
```
### With DijkstraCache routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_dijkstracache_routing.xml --hosts_count=300 --edges=2 --routing=DijkstraCache --seed=1
```

## Deployment generation
```bash
bitcoin-simgrid$ utils/createDeploymentXml --file=platform/100_nodes_deployment.xml --nodes_count=100 --peers_count=8
```
