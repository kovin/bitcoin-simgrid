#!/bin/bash

if [[ ! $1 ]]; then
    echo "Usage $0 command-to-run [command-arg1 [command-arg2 [...]]]"
    exit 1
fi
#tmpFile=/tmp/$1.log
tmpFile=$(tempfile)
"${@:1}" 2> $tmpFile &

new_job_started="$(jobs -n)"
if [ -n "$new_job_started" ];then
    pid=$!
else
    echo "process id not found"
    exit 2
fi

rss=0
realTime=0
while [ -f /proc/$pid/status ]
do
    newRssValue=$(cat /proc/$pid/status | grep -P 'VmRSS:\s+\d+ kB' | grep -oP '\d+')
    if [[ $newRssValue ]]; then
        rss=$(( $newRssValue > $rss ? $newRssValue : $rss ))
    fi
    newRealTime=$(ps -o etimes= -p $pid | grep -oP '\d+')
    if [[ $newRealTime ]]; then
        realTime=$newRealTime
    fi
    sleep 1
done

simulationTime=$(tail $tmpFile | grep -oP "\[[\d.]+\]" | grep -oP "[\d.]+")
echo "Used rss:        $rss kB"
echo "Real time:       $realTime"
echo "Simulation time: $simulationTime"

rm $tmpFile
