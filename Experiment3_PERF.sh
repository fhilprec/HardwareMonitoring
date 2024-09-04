#!/bin/bash

cd build

# Function to compute percentage deviation
compute_deviation() {
    local value1=$1
    local value2=$2
    
    if (( $(echo "$value1 == 0" | bc -l) )) && (( $(echo "$value2 == 0" | bc -l) )); then
        echo "0%"
    elif (( $(echo "$value1 == 0" | bc -l) )); then
        echo "inf%"
    else
        local deviation=$(echo "scale=2; ($value2 - $value1) * 100 / $value1" | bc)
        echo "${deviation}%"
    fi
}

# Number of iterations
iterations=1

# Arrays to store sums for PerfComp_RawDevice
declare -a raw_device_sums=(0 0 0 0 0 0 0)

# Run Experiment3_PerfComp_RawDevice
echo "Running Experiment3_PerfComp_RawDevice..."
for i in $(seq 1 $iterations); do
    output=$(./Experiment3_PerfComp_RawDevice | tail -n 1)
    IFS=',' read -ra values <<< "$output"
    for j in "${!values[@]}"; do
        raw_device_sums[$j]=$(echo "${raw_device_sums[$j]} + ${values[$j]}" | bc -l)
    done
done

# Calculate averages for PerfComp_RawDevice
declare -a raw_device_avgs
for i in "${!raw_device_sums[@]}"; do
    raw_device_avgs[$i]=$(echo "scale=2; ${raw_device_sums[$i]} / $iterations" | bc -l)
done

# Run Experiment3_PerfComp_Monitor
echo "Running Experiment3_PerfComp_Monitor..."
for i in $(seq 1 $iterations); do
    ./Experiment3_PerfComp_Monitor
done

# Read the last line from CPUPerfTwoShot.csv (which should contain the average)
monitor_output=$(tail -n 1 testOutput/CPUPerfTwoShot.csv)
IFS=';' read -ra monitor_values <<< "$monitor_output"

# Define column names
columns=("cycles" "kcycles" "instructions" "L1-misses" "LLC-misses" "branch-misses" "task-clock")

# Print header
echo "Column,PerfComp_RawDevice,PerfComp_Monitor,Deviation"

# Compare values and compute deviation
for i in "${!columns[@]}"; do
    if [ $i -lt ${#raw_device_avgs[@]} ] && [ $i -lt ${#monitor_values[@]} ]; then
        # Remove any non-numeric characters from the values
        raw_value=$(echo "${raw_device_avgs[$i]}" | tr -dc '0-9.')
        monitor_value=$(echo "${monitor_values[$i]}" | tr -dc '0-9.')
        
        if [[ -n $raw_value && -n $monitor_value ]]; then
            deviation=$(compute_deviation "$raw_value" "$monitor_value")
            echo "${columns[$i]},$raw_value,$monitor_value,$deviation"
        else
            echo "${columns[$i]},N/A,N/A,N/A"
        fi
    fi
done