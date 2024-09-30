#!/bin/bash

min_threads=7
max_threads=16+1
max_iterations=(10)
num_runtimes=20

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

    if [ $threads -ge 15 ]
    then
      echo -n "no;${iterations};${threads};${run};">>test_intense_cpu/runtimes.csv
          {  numactl -l ./TestHardwareMonitoringIntenseCPU n "$iterations" "$threads"; } >> test_intense_cpu/runtimes.csv

              rm -rf test_intense_cpu/temp_output/*
          echo -n "yes;${iterations};${threads};${run};">>test_intense_cpu/runtimes.csv
          {  numactl -l ./TestHardwareMonitoringIntenseCPU y "$iterations" "$threads"; } >> test_intense_cpu/runtimes.csv
        rm -rf test_intense_cpu/temp_output/*
    else
    echo -n "no;${iterations};${threads};${run};">>test_intense_cpu/runtimes.csv
    {  numactl -C "0-${threads+1}" -l ./TestHardwareMonitoringIntenseCPU n "$iterations" "$threads"; } >> test_intense_cpu/runtimes.csv

        rm -rf test_intense_cpu/temp_output/*
    echo -n "yes;${iterations};${threads};${run};">>test_intense_cpu/runtimes.csv
    { numactl -C "0-${threads+1}" -l ./TestHardwareMonitoringIntenseCPU y "$iterations" "$threads"; } >> test_intense_cpu/runtimes.csv

    rm -rf test_intense_cpu/temp_output/*
    fi
    done
     done
  done
