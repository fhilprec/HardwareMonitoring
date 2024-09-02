#!/bin/bash

# -----------------------------
# Part 1: Experiment3_PerfComp_Monitor
# -----------------------------

echo "Running Experiment3_PerfComp_Monitor..."
cd build

# Variables to accumulate the sum of each column
sum_cycles=0
sum_kcycles=0
sum_instructions=0
sum_L1_misses=0
sum_LLC_misses=0
sum_branch_misses=0
sum_task_clock=0
sum_scale=0
sum_IPC=0
sum_CPUs=0
sum_GHz=0

# Number of iterations
iterations=10

# Loop to execute the command 10 times
for i in $(seq 1 $iterations)
do
    # Capture the output of the command
    output=$(./Experiment3_PerfComp_PerfeventHeader| tail -n 1)

    # Parse the output and add to the corresponding sums
    cycles=$(echo $output | awk -F, '{print $1}')
    kcycles=$(echo $output | awk -F, '{print $2}')
    instructions=$(echo $output | awk -F, '{print $3}')
    L1_misses=$(echo $output | awk -F, '{print $4}')
    LLC_misses=$(echo $output | awk -F, '{print $5}')
    branch_misses=$(echo $output | awk -F, '{print $6}')
    task_clock=$(echo $output | awk -F, '{print $7}')
    scale=$(echo $output | awk -F, '{print $8}')
    IPC=$(echo $output | awk -F, '{print $9}')
    CPUs=$(echo $output | awk -F, '{print $10}')
    GHz=$(echo $output | awk -F, '{print $11}')

    # Sum the values
    sum_cycles=$(echo "$sum_cycles + $cycles" | bc)
    sum_kcycles=$(echo "$sum_kcycles + $kcycles" | bc)
    sum_instructions=$(echo "$sum_instructions + $instructions" | bc)
    sum_L1_misses=$(echo "$sum_L1_misses + $L1_misses" | bc)
    sum_LLC_misses=$(echo "$sum_LLC_misses + $LLC_misses" | bc)
    sum_branch_misses=$(echo "$sum_branch_misses + $branch_misses" | bc)
    sum_task_clock=$(echo "$sum_task_clock + $task_clock" | bc)
    sum_scale=$(echo "$sum_scale + $scale" | bc)
    sum_IPC=$(echo "$sum_IPC + $IPC" | bc)
    sum_CPUs=$(echo "$sum_CPUs + $CPUs" | bc)
    sum_GHz=$(echo "$sum_GHz + $GHz" | bc)
done

# Compute the average for each value
avg_cycles=$(echo "scale=2; $sum_cycles / $iterations" | bc)
avg_kcycles=$(echo "scale=2; $sum_kcycles / $iterations" | bc)
avg_instructions=$(echo "scale=2; $sum_instructions / $iterations" | bc)
avg_L1_misses=$(echo "scale=2; $sum_L1_misses / $iterations" | bc)
avg_LLC_misses=$(echo "scale=2; $sum_LLC_misses / $iterations" | bc)
avg_branch_misses=$(echo "scale=2; $sum_branch_misses / $iterations" | bc)
avg_task_clock=$(echo "scale=2; $sum_task_clock / $iterations" | bc)
avg_scale=$(echo "scale=2; $sum_scale / $iterations" | bc)
avg_IPC=$(echo "scale=2; $sum_IPC / $iterations" | bc)
avg_CPUs=$(echo "scale=2; $sum_CPUs / $iterations" | bc)
avg_GHz=$(echo "scale=2; $sum_GHz / $iterations" | bc)

# Print the average values
echo "Average values after $iterations iterations for Experiment3_PerfComp_Monitor:"
echo "   cycles,     kcycles,  instructions, L1-misses, LLC-misses, branch-misses,   task-clock, scale,  IPC, CPUs,  GHz"
echo "$avg_cycles, $avg_kcycles, $avg_instructions, $avg_L1_misses, $avg_LLC_misses, $avg_branch_misses, $avg_task_clock, $avg_scale, $avg_IPC, $avg_CPUs, $avg_GHz"

# -----------------------------
# Part 2: Experiment3_PerfComp_PerfEvent_Header
# -----------------------------

echo "Running Experiment3_PerfComp_PerfEvent_Header..."

# Variables to accumulate the sum of each column
sum_task_clock=0
sum_LLC_misses=0
sum_kcycles=0
sum_branch_misses=0
sum_L1_misses=0
sum_instructions=0
sum_cycles=0

# Output file path
output_file="testOutput/CPUPerfTwoShot.csv"

# Loop to execute the command 10 times
for i in $(seq 1 $iterations)
do
    # Run the program
    ./Experiment3_PerfComp_Monitor

    # Read the last line of the output file (the latest data)
    latest_data=$(tail -n 1 $output_file)

    # Parse the output and add to the corresponding sums
    task_clock=$(echo $latest_data | awk -F';' '{print $1}')
    LLC_misses=$(echo $latest_data | awk -F';' '{print $2}')
    kcycles=$(echo $latest_data | awk -F';' '{print $3}')
    branch_misses=$(echo $latest_data | awk -F';' '{print $4}')
    L1_misses=$(echo $latest_data | awk -F';' '{print $5}')
    instructions=$(echo $latest_data | awk -F';' '{print $6}')
    cycles=$(echo $latest_data | awk -F';' '{print $7}')

    # Sum the values
    sum_task_clock=$(echo "$sum_task_clock + $task_clock" | bc)
    sum_LLC_misses=$(echo "$sum_LLC_misses + $LLC_misses" | bc)
    sum_kcycles=$(echo "$sum_kcycles + $kcycles" | bc)
    sum_branch_misses=$(echo "$sum_branch_misses + $branch_misses" | bc)
    sum_L1_misses=$(echo "$sum_L1_misses + $L1_misses" | bc)
    sum_instructions=$(echo "$sum_instructions + $instructions" | bc)
    sum_cycles=$(echo "$sum_cycles + $cycles" | bc)
done

# Compute the average for each value
avg_task_clock=$(echo "scale=2; $sum_task_clock / $iterations" | bc)
avg_LLC_misses=$(echo "scale=2; $sum_LLC_misses / $iterations" | bc)
avg_kcycles=$(echo "scale=2; $sum_kcycles / $iterations" | bc)
avg_branch_misses=$(echo "scale=2; $sum_branch_misses / $iterations" | bc)
avg_L1_misses=$(echo "scale=2; $sum_L1_misses / $iterations" | bc)
avg_instructions=$(echo "scale=2; $sum_instructions / $iterations" | bc)
avg_cycles=$(echo "scale=2; $sum_cycles / $iterations" | bc)

# Get the first line (header) to print later
header=$(head -n 1 $output_file)

# Print the average values
echo "Average values after $iterations iterations for Experiment3_PerfComp_PerfEvent_Header:"
echo "$header"
echo "$avg_task_clock;$avg_LLC_misses;$avg_kcycles;$avg_branch_misses;$avg_L1_misses;$avg_instructions;$avg_cycles;;Average;"

# Now, compute the percentage-wise difference between the first value and the average value
first_data=$(sed -n '2p' $output_file)

# Parse the first values
first_task_clock=$(echo $first_data | awk -F';' '{print $1}')
first_LLC_misses=$(echo $first_data | awk -F';' '{print $2}')
first_kcycles=$(echo $first_data | awk -F';' '{print $3}')
first_branch_misses=$(echo $first_data | awk -F';' '{print $4}')
first_L1_misses=$(echo $first_data | awk -F';' '{print $5}')
first_instructions=$(echo $first_data | awk -F';' '{print $6}')
first_cycles=$(echo $first_data | awk -F';' '{print $7}')

# # Calculate the percentage difference for each value
# diff_task_clock=$(echo "scale=2; 100 * ($avg_task_clock - $first_task_clock) / $first_task_clock" | bc)
# diff_LLC_misses=$(echo "scale=2; 100 * ($avg_LLC_misses - $first_LLC_misses) / $first_LLC_misses" | bc)
# diff_kcycles=$(echo "scale=2; 100 * ($avg_kcycles - $first_kcycles) / $first_kcycles" | bc)
# diff_branch_misses=$(echo "scale=2; 100 * ($avg_branch_misses - $first_branch_misses) / $first_branch_misses" | bc)
# diff_L1_misses=$(echo "scale=2; 100 * ($avg_L1_misses - $first_L1_misses) / $first_L1_misses" | bc)
# diff_instructions=$(echo "scale=2; 100 * ($avg_instructions - $first_instructions) / $first_instructions" | bc)
# diff_cycles=$(echo "scale=2; 100 * ($avg_cycles - $first_cycles) / $first_cycles" | bc)

# # Print the percentage difference values
# echo "Percentage-wise difference from the first value:"
# echo "$diff_task_clock;$diff_LLC_misses;$diff_kcycles;$diff_branch_misses;$diff_L1_misses;$diff_instructions;$diff_cycles;;Difference;"
