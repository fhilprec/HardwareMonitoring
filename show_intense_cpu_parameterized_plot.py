import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Create the CSV data as a string
data = """monitoring;iterations;threads;run;time
no;10;100;0;3,264
yes;10;100;0;3,518
no;10;100;1;3,206
yes;10;100;1;3,517
no;10;100;2;3,299
yes;10;100;2;3,517
no;10;100;3;3,207
yes;10;100;3;3,517
no;100;100;0;31,643
yes;100;100;0;32,078
no;100;100;1;31,659
yes;100;100;1;32,084
no;100;100;2;31,743
yes;100;100;2;31,573
no;100;100;3;31,685
yes;100;100;3;32,073
"""

# Read the CSV data into a pandas DataFrame
from io import StringIO
df = pd.read_csv("cmake-build-debug/test_intense_cpu/runtimes.csv", sep=";")

# Convert 'time' from string to float (replace comma with dot)
#df['time'] = df['time'].str.replace(",", ".").astype(float)

# Group the data by 'monitoring', 'iterations', and 'threads', and calculate mean and std of 'time'
grouped = df.groupby(['monitoring', 'iterations', 'threads']).agg(
    mean_time=('time', 'mean'),
    std_time=('time', 'std')
).reset_index()

# Unique iteration values (10, 100)
iterations = grouped['iterations'].unique()

# Create a figure for subplots
fig, axes = plt.subplots(1,len(iterations), figsize=(15, 5))  # 2 subplots (for 10, 100 iterations)

# Plot for each iteration value
for i, iteration in enumerate(iterations):
    ax = axes[i]  # Select the subplot

    # Filter data for current iteration
    monitoring_yes = grouped[(grouped['monitoring'] == 'yes') & (grouped['iterations'] == iteration)]
    monitoring_no = grouped[(grouped['monitoring'] == 'no') & (grouped['iterations'] == iteration)]

    # Plotting the "no" monitoring line
    ax.errorbar(
        monitoring_no['threads'], monitoring_no['mean_time'], yerr=monitoring_no['std_time'],
        fmt='o-', label='Monitoring: No', capsize=3)

    # Plotting the "yes" monitoring line
    ax.errorbar(
        monitoring_yes['threads'], monitoring_yes['mean_time'], yerr=monitoring_yes['std_time'],
        fmt='s-', label='Monitoring: Yes', capsize=3)

    # Set axis labels
    ax.set_xlabel('Threads')
    ax.set_ylabel('Average Time')

    # Calculate percentage difference between "Monitoring: Yes" and "No"
    percentage_diff = 100 * (monitoring_yes['mean_time'].values - monitoring_no['mean_time'].values) / monitoring_yes['mean_time'].values
    total_diff = (monitoring_yes['mean_time'].values - monitoring_no['mean_time'].values)

    # Calculate the average and standard deviation of the percentage difference
    avg_total_diff = np.mean(total_diff)
    avg_percentage_diff = np.mean(percentage_diff)

    # Annotate the average percentage difference and standard deviation on the plot
    ax.text(0.5, 0.9, f'Avg Total Diff: {avg_total_diff:.2f} sec\nAvg % Diff: {avg_percentage_diff:.2f}%',
            transform=ax.transAxes, fontsize=12, verticalalignment='top',
            horizontalalignment='center', bbox=dict(facecolor='white', alpha=0.7))


    # Set title and show legend
    ax.set_title(f'Iteration = {iteration}')
    ax.legend()

# Set overall title
plt.suptitle('Average Time vs Threads and Monitoring (2D Plots)', fontsize=16)

# Show the plot
plt.tight_layout()
plt.show()
