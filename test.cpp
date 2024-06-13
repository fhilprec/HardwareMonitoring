#include <iostream>
#include "PerfEvent.hpp"
#include "Device.hpp"
#include "PerfEvent.hpp"
//sysctl -w kernel.perf_event_paranoid=-1
//g++ test.cpp ; ./a.out ; rm a.out;

int main(){
    PerfEvent perf;
    CPUPerf cpu;
    cpu.printVector(cpu.getData());
    int ans = 0;
    perf.startCounters();
    for (int i = 0; i < 10000; i++) {
        ans = i + 6;
        ans = ans * ans * ans;
    }
    cpu.printVector(cpu.getData());
    perf.stopCounters();
    perf.printReport(std::cout, 100);
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