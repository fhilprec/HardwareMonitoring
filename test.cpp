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

    std::vector<std::unique_ptr<Device>> devices;
    devices.emplace_back(new CPUPerf());
    std::filesystem::path outputDirectory("testOutput");
    auto fullPath = absolute(outputDirectory);
    std::chrono::milliseconds pollingTime = std::chrono::milliseconds(100);

    Counter counter(devices,pollingTime, fullPath);

    counter.start();

    long long max=0;
    int maxI=0;
    int ans = 0;

    for (int i = 0; i < 10000; i++) {
        ans = i + 6;
        ans = ans * ans * ans;
    }
    std::cout << "longest Collatz Path to 1: " << max << ", starting with number: " << maxI  <<std::endl;
    counter.stop();
}

