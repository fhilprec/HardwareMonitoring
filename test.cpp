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
    std::vector<std::pair<Metric, Measurement>> data = cpu.getData(TWO_SHOT);

    for (auto& d : data) {
        std::cout << d.first.name << ": " << d.second.value << std::endl;
    }

    int ans = 0;
    
    for (int i = 0; i < 10000; i++) {
        ans = i + 6;
        ans = ans * ans * ans;
    }
    data = cpu.getData(TWO_SHOT);

    for (auto& d : data) {
        std::cout << d.first.name << ": " << d.second.value << std::endl;
    }
    //perf.stopCounters();
    //perf.printReport(std::cout, 1);
}

