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

    Counter counter(devices);
    counter.start();
    int i;
    for (i = 0; i < 1000000; i++) {
        std::cout << i << "\n";
    }

    int ans = 0;

    for (int i = 0; i < 10000; i++) {
        ans = i + 6;
        ans = ans * ans * ans;
    }
    std::cout << i << std::endl;
    counter.stop();
}

