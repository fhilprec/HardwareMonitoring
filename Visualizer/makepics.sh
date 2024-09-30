#!/bin/bash
find . -type f \(  -iname \*.csv \) -delete;
scp c02.lab.dm.informatik.tu-darmstadt.de:/home/fhilprecht/HardwareMonitoring/build/testOutput/*.csv .;
find . -type f \(   -iname \*_raw.csv \) -delete;
#blocking
# python visualizer.py --cpu "cycles" --gpu "raw_reads_n" --io "wchar"  --gpu_comp "gpu_temperature"   --scale "instructions:0.01" "raw_reads_n:450000" "wchar:0.090" "cycles:0.45" "rchar:10" "gpu_temperature:0.00000000001"    --output blocking
#pipelined
python visualizer.py --cpu "cycles" --gpu "raw_reads_n" --io "wchar"  --gpu_comp "gpu_temperature"   --scale "instructions:0.01" "raw_reads_n:90000000" "wchar:0.05" "cycles:1.9" "rchar:3000" "gpu_temperature:0.00000000001"    --output pipelined 
