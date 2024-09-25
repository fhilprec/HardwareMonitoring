#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "Device.hpp"
#include "Monitor.h"

void doWork(){}

int main() {
    std::vector<std::shared_ptr<IDevice>> devices;

    auto* device = new CPUPerf();
    devices.emplace_back(device);

    std::filesystem::path outputDirectory("output");

    MonitorConfig config = {devices, std::chrono::milliseconds(500), outputDirectory, outputDirectory};
    Monitor monitor(config);

    monitor.start();
    doWork();
    monitor.stop();
}