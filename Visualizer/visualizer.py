import argparse
import csv
from datetime import datetime
import matplotlib.pyplot as plt

#python visualizer.py --cpu "cycles" --gpu "raw_reads_n" --io "rchar" "wchar" --scale "instructions:0.01" "raw_reads_n:1000" "wchar:0.0001" "cycles:0.001"    --output my_diagram.png

def parse_timestamp(timestamp_str):
    # Remove any extra digits beyond microseconds
    timestamp_str = timestamp_str[:26]
    return datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S.%f')

def read_csv_data(file_path, columns, scale_factors):
    data = {col: [] for col in columns}
    timestamps = []

    

    with open(file_path, 'r') as csvfile:
        reader = csv.DictReader(csvfile, delimiter=';')
        
        for row in reader:
            timestamp = parse_timestamp(row['Time of Polling'])
            timestamps.append(timestamp)
            for col in columns:
                try:
                    value = float(row[col])
                    if col in scale_factors:
                        value *= scale_factors[col]
                    data[col].append(value)
                except ValueError:
                    data[col].append(None)
    
    return timestamps, data


def plot_data(timestamps, data_dict, output_file, label_step=50):
    plt.figure(figsize=(12, 6))

    # Calculate time elapsed in seconds from the first timestamp
    timestamps_elapsed = [(timestamp - timestamps[0]).total_seconds() for timestamp in timestamps]

    # Convert time elapsed to strings (seconds with two decimal precision)
    timestamps_formatted = [f'{elapsed:.2f}' for elapsed in timestamps_elapsed]

    # Plot all the data points
    i = 0
    datalabels = ['CPU', 'GPU IO Reads', 'GPU Utilization', 'SSD Writes']
    for col, values in data_dict.items():
        plt.plot(timestamps_formatted, values, label=datalabels[i])
        i += 1

    # Reduce the number of x-axis labels to every `label_step`th timestamp
    xticks_positions = range(0, len(timestamps_formatted), label_step)
    
    # Display only the seconds on x-axis labels
    xticks_labels = [f'{timestamps_elapsed[pos]:.0f}' for pos in xticks_positions]  # Show integers for seconds
    
    plt.xticks(xticks_positions, xticks_labels, fontsize='large')

    plt.ylabel('Utilization')
    plt.legend(fontsize='large', ncol=4, bbox_to_anchor=(0, 1.01, 1, 0), loc='lower left', mode="expand", borderaxespad=0.)
    plt.tight_layout()

    # Remove y-axis ticks and labels
    plt.yticks([])

    # Save the figure
    plt.savefig(output_file + '.png')
    print(f"Diagram saved as {output_file}")




    
def parse_scale_factors(scale_args):
    scale_factors = {}
    for scale_arg in scale_args:
        try:
            col, factor = scale_arg.split(':')
            scale_factors[col] = float(factor)
        except ValueError:
            print(f"Warning: Invalid scale factor '{scale_arg}'. Expected format 'column:factor'.")
    return scale_factors

def main():
    parser = argparse.ArgumentParser(description="Generate performance diagram from CSV files.")
    parser.add_argument('--cpu', nargs='*', help='Columns to plot from CPUPerf.csv')
    parser.add_argument('--gpu', nargs='*', help='Columns to plot from GPUFile.csv')
    parser.add_argument('--gpu_comp', nargs='*', help='Columns to plot from GPUCOMP.csv')
    parser.add_argument('--io', nargs='*', help='Columns to plot from IOFile.csv')
    parser.add_argument('--output', default='performance_diagram.png', help='Output file name')
    parser.add_argument('--scale', nargs='*', help='Scale factors for columns (format: column:factor)')
    
    args = parser.parse_args()
    
    scale_factors = parse_scale_factors(args.scale or [])
    
    all_timestamps = []
    all_data = {}
    
    if args.cpu:
        cpu_timestamps, cpu_data = read_csv_data('CPUPerf.csv', args.cpu, scale_factors)
        all_timestamps = cpu_timestamps
        all_data.update(cpu_data)
    
    if args.gpu:
        gpu_timestamps, gpu_data = read_csv_data('GPUFile.csv', args.gpu, scale_factors)
        if not all_timestamps:
            all_timestamps = gpu_timestamps
        all_data.update(gpu_data)
    
    if args.gpu_comp:
        gpu_comp_timestamps, gpu_comp_data = read_csv_data('GPUComputation.csv', args.gpu_comp, scale_factors)
        if not all_timestamps:
            all_timestamps = gpu_comp_timestamps
        all_data.update(gpu_comp_data)
    
    if args.io:
        io_timestamps, io_data = read_csv_data('IOFile.csv', args.io, scale_factors)
        if not all_timestamps:
            all_timestamps = io_timestamps
        all_data.update(io_data)
    
    if not all_data:
        print("No data to plot. Please specify at least one column from one file.")
        return
    
    plot_data(all_timestamps, all_data, args.output )

if __name__ == "__main__":
    main()