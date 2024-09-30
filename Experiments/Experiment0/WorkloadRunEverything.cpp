#include <vector>

#include "CPUPerf.h"
#include "Monitor.h"
#include "GPUFile.h"
#include "NIC.h"


void doWork(){}

int main() {
    std::vector<std::shared_ptr<IDevice>> devices;

    auto* device = new IOFile();
    devices.emplace_back((IDevice*)device);
    auto* device2 = new CPUPerf();
    devices.emplace_back((IDevice*)device2);
    auto* gpuDevice = new GPUFile();
    devices.emplace_back((IDevice*)gpuDevice);
    // auto* nic = new NIC("mlx5_0", 1);
    // devices.emplace_back((IDevice*)nic);

    std::filesystem::path outputDirectory("testOutput");
    auto fullPath = absolute(outputDirectory);

    // Create the Monitor object with the correct constructor signature
    Monitor monitor({devices,std::chrono::milliseconds(500),outputDirectory, outputDirectory});

    // Note: We can't set the polling time here as the constructor doesn't accept it.
    // If you need to set a custom polling time, you might need to modify the Monitor class.

    monitor.start();

    // Perform some CPU and I/O work
    const std::string filename = "test_file.bin";
    const int fileSizeMB = 50; // 50 MB file

    std::cout << "Writing to file..." << std::endl;
    writeToFile(filename, fileSizeMB);

    std::cout << "Reading from file..." << std::endl;
    readFromFile(filename);

    std::cout << "Performing CPU work..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        doCPUWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Performing simulated GPU work..." << std::endl;
    doSimulatedGPUWork(1000000); // 1 million elements
    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    monitor.stop();

    // Clean up the test file
    std::filesystem::remove(filename);

    std::cout << "Test completed. Check the testOutput directory for results." << std::endl;

    return 0;
}