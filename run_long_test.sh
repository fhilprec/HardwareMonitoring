#!/bin/bash

rm -rf test_long_time
mkdir test_long_time

touch test_long_time/mem.log

./TestHardwareMonitoringLongTime &
pid="$(pidof TestHardwareMonitoringLongTime)"

trap "kill $pid 2> /dev/null" EXIT

while ps -p "$pid" &> /dev/null;
do
    echo -n "$(date -u)" >> test_long_time/mem.log
    cat "/proc/$pid/statm" >> test_long_time/mem.log
    sleep 10
done

trap - EXIT