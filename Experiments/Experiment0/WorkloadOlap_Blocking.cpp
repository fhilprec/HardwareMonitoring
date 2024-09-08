#include <iostream>
#include <vector>
#include <memory>
#include <filesystem>
#include <chrono>
#include <thread>
#include <stdexcept>

#include "Device.hpp"
#include "CPUPerf.h"
#include "IOFile.h"
#include "Monitor.h"
#include "GPUFile.h"
#include "NIC.h"

// Declare the CUDA workload function
extern "C" void runCudaWorkload(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    //     std::vector<std::shared_ptr<IDevice>> devices;

    //    auto* device = new IOFile();
    // devices.emplace_back((IDevice*)device);
    // auto* device2 = new CPUPerf();
    // devices.emplace_back((IDevice*)device2);
    // auto* gpuDevice = new GPUFile();
    // devices.emplace_back((IDevice*)gpuDevice);
    // auto* nic = new NIC();
    // devices.emplace_back((IDevice*)nic);

    //     std::filesystem::path outputDirectory("testOutput");
    //     auto fullPath = std::filesystem::absolute(outputDirectory);

    //     // Create the Monitor object
    //     MonitorConfig config{
    //         devices,
    //         std::chrono::milliseconds(500),
    //         outputDirectory,
    //         outputDirectory
    //     };
    //     Monitor monitor(config);

    //     // Start monitoring
    //     std::cout << "Starting monitoring..." << std::endl;
    //     monitor.start();

        // Run the CUDA workload
        std::cout << "Starting CUDA workload..." << std::endl;
        runCudaWorkload(argc, argv);
        std::cout << "CUDA workload completed." << std::endl;

        // // Stop monitoring
        // std::cout << "Stopping monitoring..." << std::endl;
        // monitor.stop();

    

    return 0;
}