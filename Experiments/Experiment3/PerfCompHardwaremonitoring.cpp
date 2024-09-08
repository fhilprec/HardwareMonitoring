#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "Device.hpp"
#include "CPUPerfTwoShot.h"
#include "Monitor.h"




// Function to perform some CPU-intensive work
void doCPUWork() {
    volatile int sum = 0;
    for (int i = 0; i < 10000000; ++i) {
        sum += i * i;
    }
}



int main() {
    std::vector<std::shared_ptr<IDevice>> devices;

    auto* device2 = new CPUPerfTwoShot();
    devices.emplace_back((IDevice*)device2);

    std::filesystem::path outputDirectory("testOutput");
    auto fullPath = absolute(outputDirectory);

    // Create the Monitor object with the correct constructor signature
    Monitor monitor({devices,std::chrono::milliseconds(500),outputDirectory, outputDirectory});

    // Note: We can't set the polling time here as the constructor doesn't accept it.
    // If you need to set a custom polling time, you might need to modify the Monitor class.

    monitor.start();

    
    for (int i = 0; i < 20; ++i) {
        doCPUWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    monitor.stop();



    return 0;
}