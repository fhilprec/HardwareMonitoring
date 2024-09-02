#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "Device.hpp"
#include "PerfEvent.hpp"
#include "Monitor.h"




// Function to perform some CPU-intensive work
void doCPUWork() {
    volatile int sum = 0;
    for (int i = 0; i < 10000000; ++i) {
        sum += i * i;
    }
}



int main() {
    
    PerfEvent e;
    e.startCounters();

    
    std::cout << "Performing CPU work..." << std::endl;
    for (int i = 0; i < 20; ++i) {
        doCPUWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    e.stopCounters();
    e.printReport(std::cout, 1); // use n as scale factor




    return 0;
}