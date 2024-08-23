import argparse
import csv
from datetime import datetime
import matplotlib.pyplot as plt

def parse_timestamp(timestamp_str):
    return datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S.%f')

def read_csv_data(file_path, columns):
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
                    data[col].append(value)
                except ValueError:
                    data[col].append(None)
    
    return timestamps, data

def plot_data(timestamps, data_dict, output_file):
    plt.figure(figsize=(12, 6))
    for col, values in data_dict.items():
        plt.plot(timestamps, values, label=col)
    
    plt.xlabel('Time')
    plt.ylabel('Operations')
    plt.title('Performance Metrics Over Time')
    plt.legend()
    plt.xticks(rotation=45)
    plt.tight_layout()
    
    plt.savefig(output_file)
    print(f"Diagram saved as {output_file}")

def main():
    parser = argparse.ArgumentParser(description="Generate performance diagram from CSV files.")
    parser.add_argument('--cpu', nargs='*', help='Columns to plot from CPUPerf.csv')
    parser.add_argument('--gpu', nargs='*', help='Columns to plot from GPUFile.csv')
    parser.add_argument('--io', nargs='*', help='Columns to plot from IOFile.csv')
    parser.add_argument('--output', default='performance_diagram.png', help='Output file name')
    
    args = parser.parse_args()
    
    all_timestamps = []
    all_data = {}
    
    if args.cpu:
        cpu_timestamps, cpu_data = read_csv_data('CPUPerf.csv', args.cpu)
        all_timestamps = cpu_timestamps
        all_data.update(cpu_data)
    
    if args.gpu:
        gpu_timestamps, gpu_data = read_csv_data('GPUFile.csv', args.gpu)
        if not all_timestamps:
            all_timestamps = gpu_timestamps
        all_data.update(gpu_data)
    
    if args.io:
        io_timestamps, io_data = read_csv_data('IOFile.csv', args.io)
        if not all_timestamps:
            all_timestamps = io_timestamps
        all_data.update(io_data)
    
    if not all_data:
        print("No data to plot. Please specify at least one column from one file.")
        return
    
    plot_data(all_timestamps, all_data, args.output)

if __name__ == "__main__":
    main()