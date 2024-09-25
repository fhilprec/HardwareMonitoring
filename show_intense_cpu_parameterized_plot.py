import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Read the CSV data into a pandas DataFrame
df = pd.read_csv("cmake-build-debug/test_intense_cpu_5/runtimes.csv", sep=";")

# Convert 'time' from string to float (replace comma with dot)
#df['time'] = df['time'].str.replace(",", ".").astype(float)
print(sum(df['time'])/(60))
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

# Plot for each iteration value
for i, iteration in enumerate(iterations):
    # Filter data for the current iteration
    monitoring_yes = grouped[(grouped['monitoring'] == 'yes') & (grouped['iterations'] == iteration)]
    monitoring_no = grouped[(grouped['monitoring'] == 'no') & (grouped['iterations'] == iteration)]

    # Calculate the percentage difference between "Monitoring: Yes" and "Monitoring: No"
    percentage_diff = 100*(monitoring_yes['median_time'].values - monitoring_no['median_time'].values) / monitoring_yes['median_time'].values

    # Plotting the bars for the percentage differences
    ax.bar(index + i * bar_width, percentage_diff, bar_width, color=colors[i], label=f'{('Small Workload', 'Normal Workload', 'Big Workload')[i]}')

    # Add error bars based on the standard deviations of "Yes" and "No"
    # Add error bars based on the standard deviations of "Yes" and "No"
# Set axis labels and title
ax.set_xlabel('Threads')
ax.set_ylabel('Time Difference (%)')
ax.set_title('Monitoring Slow Down (5ms Sampling)')

# Set x-axis ticks
ax.set_xticks(index + (len(iterations) - 1) * bar_width / 2)
ax.set_xticklabels(threads)

# Add a horizontal line at zero to indicate no difference
ax.axhline(0, color='gray', linestyle='--')

# Add a legend to distinguish between iterations
ax.legend()

# Adjust layout and display the plot
plt.tight_layout()
plt.show()

# Save the figure as a PNG file
#fig.savefig("/home/aleks-tu/cpu_intense_percent_plot.png")
