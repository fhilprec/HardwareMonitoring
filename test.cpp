#include <iostream>
#include "PerfEvent.hpp"
#include "Device.hpp"
#include "PerfEvent.hpp"
//sysctl -w kernel.perf_event_paranoid=-1
//g++ test.cpp ; ./a.out ; rm a.out;

int main(){
    //PerfEvent perf;
    CPUPerf cpu;
    //perf.startCounters();
    cpu.start();
    int ans = 0;
    
    for (int i = 0; i < 10000; i++) {
        ans = i + 6;
        ans = ans * ans * ans;
    }
    cpu.stop();
    cpu.printVector();
    //perf.stopCounters();
    //perf.printReport(std::cout, 1);
}

