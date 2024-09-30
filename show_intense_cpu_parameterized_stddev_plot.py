import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Create the CSV data as a string
data = """monitoring;iterations;threads;run;time
no;10;100;0;3264
yes;10;100;0;3518
no;10;100;1;3206
yes;10;100;1;3517
no;10;100;2;3299
yes;10;100;2;3517
no;10;100;3;3207
yes;10;100;3;3517
no;100;100;0;31643
yes;100;100;0;32078
no;100;100;1;31659
yes;100;100;1;32084
no;100;100;2;31743
yes;100;100;2;31573
no;100;100;3;31685
yes;100;100;3;32073
"""

# Read the CSV data into a pandas DataFrame
from io import StringIO
df = pd.read_csv("final_cpu/runtimes.csv", sep=";")

# Group the data by 'monitoring', 'iterations', and 'threads', and calculate median and std of 'time'
grouped = df.groupby(['monitoring', 'iterations', 'threads']).agg(
    median_time=('time', 'median'),
    std_time=('time', 'std')
).reset_index()

# Unique iteration values (e.g., 10, 100, ...)
iterations = grouped['iterations'].unique()

# Unique thread values
threads = grouped['threads'].unique()

# Create a figure for the bar chart
fig, ax = plt.subplots(figsize=(10, 6))

# Bar width and position shift for grouped bars
bar_width = 0.2
index = np.arange(len(threads))  # x-axis positions for thread values

# Define colors for different iterations
colors = plt.cm.viridis(np.linspace(0, 1, len(iterations)))

# Variable to store all percentage standard deviations
all_percentage_std_diffs = []

# Plot for each iteration value
for i, iteration in enumerate(iterations):
    # Filter data for the current iteration
    monitoring_yes = grouped[(grouped['monitoring'] == 'yes') & (grouped['iterations'] == iteration)]
    monitoring_no = grouped[(grouped['monitoring'] == 'no') & (grouped['iterations'] == iteration)]

    # Calculate the absolute standard deviation of the time difference
    std_diff = np.sqrt(monitoring_yes['std_time'].values**2 + monitoring_no['std_time'].values**2)

    # Convert absolute standard deviation to percentage of the "Monitoring: Yes" median time
    percentage_std_diff = 100 * std_diff / monitoring_yes['median_time'].values
    if(i==2): all_percentage_std_diffs.extend(percentage_std_diff)  # Store the percentage standard deviations

    # Plotting the bars for the standard deviation percentage
    ax.bar(index + i * bar_width, percentage_std_diff, bar_width, color=colors[i], label=f'Iter = {iteration}')

    # Annotate the bar values with the percentage standard deviation
    for j, d in enumerate(percentage_std_diff):
        ax.text(index[j] + i * bar_width, d + 0.5, f'{d:.2f}%',
                fontsize=10, ha='center', va='bottom')

# Set axis labels and title
ax.set_xlabel('Threads')
ax.set_ylabel('Std Dev of Time Difference (%)')
ax.set_title('Standard Deviation of Time Difference as Percentage (Monitoring Yes - No)')
# Calculate and print the average std dev percentage difference
avg_percentage_std_diff = np.mean(all_percentage_std_diffs)
print(f'Average Standard Deviation Percentage Difference: {avg_percentage_std_diff:.2f}%')
# Set x-axis ticks
ax.set_xticks(index + (len(iterations) - 1) * bar_width / 2)
ax.set_xticklabels(threads)

# Add a legend to distinguish between iterations
ax.legend()

# Adjust layout and display the plot
plt.tight_layout()
plt.show()

# Save the figure as a PNG file
fig.savefig("/home/aleks-tu/cpu_intense_stddev_percent_plot.png")

