#!/bin/bash
find . -type f \( -iname \*.png -o -iname \*.csv \) -delete;
scp c02.lab.dm.informatik.tu-darmstadt.de:/home/fhilprecht/HardwareMonitoring/build/testOutput/*.csv .;
find . -type f \( -iname \*.png -o -iname \*_raw.csv \) -delete;
python visualizer.py --cpu "cycles" --gpu "raw_reads_n" --io "rchar" "wchar" --scale "instructions:0.01" "raw_reads_n:1000" "wchar:0.0001" "cycles:0.001"    --output my_diagram.png

