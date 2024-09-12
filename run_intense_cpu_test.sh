#!/bin/bash

rm -rf test_intense_cpu
mkdir test_intense_cpu

# get runtimes
touch test_intense_cpu/without_monitoring.log
echo "Process Time:" > test_intense_cpu/without_monitoring.log
{ time ./TestHardwareMonitoringIntenseCPU ; } 2>> test_intense_cpu/without_monitoring.log

touch test_intense_cpu/with_monitoring.log
echo "Process Time:" > test_intense_cpu/with_monitoring.log
{ time ./TestHardwareMonitoringIntenseCPU useMonitoring ; } 2>> test_intense_cpu/with_monitoring.log

#check cpu/memory/io in comparison
./TestHardwareMonitoringIntenseCPU &
pid="$(pidof TestHardwareMonitoringIntenseCPU)"
trap "kill $pid 2> /dev/null" EXIT

while ps -p "$pid" &> /dev/null;
do
    echo "Time: $(date -u)" >> test_intense_cpu/without_monitoring.log
        echo "Memory: $(cat "/proc/$pid/statm") " >> test_intense_cpu/without_monitoring.log
        echo "Stat: $(cat "/proc/$pid/stat") | " >> test_intense_cpu/without_monitoring.log
    iostat >> test_intense_cpu/without_monitoring.log
    echo "------------------------------------------------------------------" >> test_intense_cpu/without_monitoring.log
    sleep 1
done

trap - EXIT

./TestHardwareMonitoringIntenseCPU useMonitoring &
pid="$(pidof TestHardwareMonitoringIntenseCPU)"
trap "kill $pid 2> /dev/null" EXIT


while ps -p "$pid" &> /dev/null;
do
    echo "Time: $(date -u)" >> test_intense_cpu/with_monitoring.log
    echo "Memory: $(cat "/proc/$pid/statm") " >> test_intense_cpu/with_monitoring.log
    echo "Stat: $(cat "/proc/$pid/stat") | " >> test_intense_cpu/with_monitoring.log
    iostat >> test_intense_cpu/with_monitoring.log
    echo "------------------------------------------------------------------" >> test_intense_cpu/with_monitoring.log
    sleep 1
done

trap - EXIT