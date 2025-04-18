#!/usr/bin/env bash

# $1 = number to calc

# $2 = number of processes

pids=()

pushd testPrograms

for (( i=1 ; i<=$2 ; i++ )); 
do  

    python ./badfib.py $1 &
    pids+=($!)
    echo ${pids[*]}

done

for pid in ${pids[@]};
do
    wait $pid

done

popd