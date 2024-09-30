#!/bin/bash

# Change to the build directory
cd build

# Function to compute percentage deviation
compute_deviation() {
    local value1=$1
    local value2=$2
    
    if [ "$value1" -eq 0 ] && [ "$value2" -eq 0 ]; then
        echo "0%"
    elif [ "$value1" -eq 0 ]; then
        echo "inf%"
    else
        local deviation=$(( (value2 - value1) * 100 / value1 ))
        echo "${deviation}%"
    fi
}
./Experiment3_IOComp_Monitor
./Experiment3_IOComp_RawDevice

# Read the last line from both files
line1=$(tail -n 1 testOutput/io_stats.csv)
line2=$(tail -n 1 testOutput/IOFileTwoShot.csv)

# Split the lines into arrays
IFS=';' read -ra values1 <<< "$line1"
IFS=';' read -ra values2 <<< "$line2"

# Define column names
columns=("rchar" "wchar" "syscr" "syscw" "read_bytes" "write_bytes" "cancelled_write_bytes")

# Print header
echo "Column,io_stats.csv,IOFileTwoShot.csv,Deviation"

# Compare values and compute deviation
for i in "${!columns[@]}"; do
    if [ $i -lt ${#values1[@]} ] && [ $i -lt ${#values2[@]} ]; then
        deviation=$(compute_deviation "${values1[$i]}" "${values2[$i]}")
        echo "${columns[$i]},${values1[$i]},${values2[$i]},$deviation"
    fi
done