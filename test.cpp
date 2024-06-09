#include <iostream>
#include "Device.hpp"
#include "Counter.hpp"
//sysctl -w kernel.perf_event_paranoid=-1
//g++ test.cpp ; ./a.out ; rm a.out;

int main() {
    /* CPUPerf cpu;
     cpu.printVector(cpu.getData());
     int i = 0;
     for (int i = 0; i < 100; i++) {
         i = i + 1;
         i = i * i;
     }
     cpu.printVector(cpu.getData());*/

    std::vector<Device> devices;
    devices.push_back(CPUPerf());

    std::chrono::milliseconds pollingDuration(100);

    Counter counter(devices, pollingDuration);
    counter.start();
    int i;
    for (i = 0; i < 1000000; i++) {
        std::cout << i << "\n";
    }
    std::cout << i << std::endl;
    counter.stop();
}

/*
int main(){
    int a = 7;
    int n = 10000;
    PerfEvent e;
    e.startCounters();
    for (int i=0; i<n; i++) // this code will be measured
    a = a * a;
    e.stopCounters();
    e.printReport(std::cout, n); / / use n as scale factor
    std::cout << std::endl;

    CSVWriter csv;
    csv.newRow() << "this" << "is" << "the" << "first" << "row";
    csv.newRow() << "this" << "is" << "the" << "second" << "row";
    csv.writeToFile("foobar.csv");
    return 0;
}*/