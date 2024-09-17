#!/bin/bash
find . -type f \(  -iname \*.csv \) -delete;
scp c02.lab.dm.informatik.tu-darmstadt.de:/home/fhilprecht/HardwareMonitoring/build/testOutput/*.csv .;
find . -type f \(   -iname \*_raw.csv \) -delete;
#blocking
python visualizer.py --cpu "cycles" --gpu "raw_reads_n" --io "rchar" "wchar"  --gpu_comp "gpu_utilization"   --scale "instructions:0.01" "raw_reads_n:100000" "wchar:0.1" "cycles:1" "rchar:10" "gpu_utilization:0.00000000001"    --output my_diagram.png 