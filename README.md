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
### Simple
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default_platform.xml platform/default_deployment/

```
### Setting the simulation duration time (in seconds)
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default_platform.xml platform/default_deployment/ --duration-time=30000

```
### With debug information
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default_platform.xml platform/default_deployment/ -debug

```
### With custom log
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default_platform.xml platform/default_deployment/ --log="bitcoin_simgrid.fmt:%d%10h:%e%m%n bitcoin_simgrid.thres:debug"

```

## Topology generation
### With Full routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_full_routing.xml --hosts_count=300 --edges=8 --routing=Full --seed=1
```
### With Dijkstra routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_dijkstra_routing.xml --hosts_count=300 --edges=8 --routing=Dijkstra --seed=1
```
### With DijkstraCache routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/300_nodes_dijkstracache_routing.xml --hosts_count=300 --edges=8 --routing=DijkstraCache --seed=1
```

## Deployment generation
```bash
bitcoin-simgrid$ utils/createDeploymentXml --nodes_count=300 --peers_count=8 --data_dir=platform/300_nodes_deployment --miners_ratio=10 --txs_per_day=200000 --difficulty=3462542391191
```
