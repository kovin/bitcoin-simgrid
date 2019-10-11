# bitcoin-simgrid

## About
This project provides a simulation of the Bitcoin network using SimGrid.

Currently we are modeling nodes, miners, transactions and blocks.
We're also offering tools to generate the physical platform (ie: how many host and the physical links among them) as well as the logical platform (ie: how many Bitcoin nodes and to what peers they are connected).

## Prerequisites
```bash
$ cd ~
$ sudo apt-get install doxygen libboost-all-dev cmake valgrind python-networkx python-lxml git
$ git clone --branch v3_21 --depth 1 https://framagit.org/simgrid/simgrid.git simgrid
$ cd simgrid
simgrid$ cmake -DCMAKE_INSTALL_PREFIX=/opt/simgrid .
simgrid$ make simgrid
simgrid$ sudo make install
```

## Clone
```bash
$ cd ~
$ git clone git@gitlab.licar.exp.dc.uba.ar:licar/bitcoin-simgrid.git
$ cd bitcoin-simgrid
```

## Build
```bash
bitcoin-simgrid$ cmake .
bitcoin-simgrid$ make
```

## Run

### Usage
```bash
bin/bitcoin_simgrid platform_file deployment_directory [--simulation-duration <seconds>] [--target-time <seconds>] [--sleep-duration <milliseconds>] [--custom-log]
```
Options:
* --simulation-duration: for how long do you want to run the simulation. By default 3600 seconds (1 hour)
* --target-time: how long should you expect blocks to be mined. By default 600 seconds (10 minutes). Initially this depends on the current difficulty of the network and the available hash power. Every 2016 blocks, the network difficulty will be adapted to match this target time.
* --sleep-duration: for how long should nodes go to sleep when there're no more message to process. By default 100 milliseconds.
* --seed: allow to set random generator seed in order to reproduce simulations in a deterministic way
* --custom-log: if you use this flag then you can use native SimGrid option --log.
* --hashrate-scale: JSON encoded number are more limited than C++ ones and can't represent legitimate high values. So the tool accepts lower JSON encoded hashrate values that can then be up-scaled using this argument
* --skip-time-when-possible: if true, then we will avoid the loop events of each node when we know there are no more messages to receive/send until the next global activity in the network
* --debug: if true, more information about transactions and blocks will be included in the produced log

### Simple
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default/platform.xml platform/default/deployment/

```
### Setting the simulation duration time (in seconds). If not specified it will be 3600 by default
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default/platform.xml platform/default/deployment/ --simulation-duration 7200

```
### With custom log
Read about possible log options at http://simgrid.gforge.inria.fr/simgrid/3.20/doc/outcomes_logs.html#log_user
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default/platform.xml platform/default/deployment/ --custom-log --log="bitcoin_simgrid.fmt:%c%e%d%10h:%e%m%n bitcoin_simgrid.thres:info"

```
### To reproduce a real blockchain trace
```bash
bitcoin-simgrid$ bin/bitcoin-simgrid platform/default/platform.xml platform/trace_deployment/

```


## Topology generation

### With DijkstraCache routing (recommended)
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/default/platform.xml --hosts_count=300 --edges=8 --routing=DijkstraCache --seed=1
```

### With Dijkstra routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/default/platform.xml --hosts_count=300 --edges=8 --routing=Dijkstra --seed=1
```

### With Full routing
```bash
bitcoin-simgrid$ utils/createPlatformXml --file=platform/default/platform.xml --hosts_count=300 --edges=8 --routing=Full --seed=1
```

## Deployment generation
```bash
bitcoin-simgrid$ utils/createDeploymentXml --nodes_count=300 --peers_count=8 --data_dir=platform/default/deployment --miners_ratio=10 --txs_per_day=200000 --difficulty=3462542391191 --global_hashrate=25130091717 --distribution_type=exponential --distribution_lambda=2.5 --seed=1
```

## Deployment generation with trace
```bash
bitcoin-simgrid$ utils/createDeploymentXml --nodes_count=300 --peers_count=8 --data_dir=platform/trace_deployment --difficulty=3462542391191 --distribution_type=exponential --distribution_lambda=2.5 --trace_dir=blockchain --activity_generation_type=trace --seed=1
```
