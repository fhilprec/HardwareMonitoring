#!/bin/bash

min_threads=1
max_threads=16+1
max_iterations=(10 100 1000)
num_runtimes=3

rm -rf test_intense_cpu
mkdir test_intense_cpu
mkdir test_intense_cpu/temp_output
touch test_intense_cpu/runtimes.csv
echo "monitoring;iterations;threads;run;time">>test_intense_cpu/runtimes.csv

TIMEFORMAT=%R

for ((threads=min_threads;threads<=max_threads;threads+=1));
 do
   for iterations in "${max_iterations[@]}"
   do
     for((run=0;run<=num_runtimes;run+=1));
     do

    # get runtimes
    echo -n "no;${iterations};${threads};${run};">>test_intense_cpu/runtimes.csv
    { time ./TestHardwareMonitoringIntenseCPU n "$iterations" "$threads"; } 2>> test_intense_cpu/runtimes.csv

        rm -rf test_intense_cpu/temp_output/*
    echo -n "yes;${iterations};${threads};${run};">>test_intense_cpu/runtimes.csv
    { time ./TestHardwareMonitoringIntenseCPU y "$iterations" "$threads"; } 2>> test_intense_cpu/runtimes.csv

    rm -rf test_intense_cpu/temp_output/*
    done
     done
  done
