#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "Device.hpp"
#include "CPUPerf.h"
#include "IOFile.h"
#include "Monitor.h"
#include "GPUFile.h"


// Function to simulate GPU work (vector addition)
void simulateGPUWork(const std::vector<float>& A, const std::vector<float>& B, std::vector<float>& C)
{
    for (size_t i = 0; i < A.size(); ++i)
    {
        C[i] = A[i] + B[i];
    }
}

// Function to perform some simulated GPU work
void doSimulatedGPUWork(int numElements)
{
    std::vector<float> h_A(numElements);
    std::vector<float> h_B(numElements);
    std::vector<float> h_C(numElements);

    // Initialize input vectors
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < numElements; ++i)
    {
        h_A[i] = dis(gen);
        h_B[i] = dis(gen);

        if(i % (numElements/5) == 0){
            int ret = system("/usr/local/cuda-12/gds/tools/gdsio -d 0 -w 4 -s 4G -i 1M -x 0 -I 0 -f /dev/md127");
            // Check if the command executed successfully
            if (ret != 0)
            {
                std::cerr << "Command execution failed at element " << i << std::endl;
                break;
            }
        }
    }

    // Perform the simulated GPU work
    simulateGPUWork(h_A, h_B, h_C);

    // Verify the result (optional)
    for (int i = 0; i < numElements; ++i)
    {
        if (std::abs(h_A[i] + h_B[i] - h_C[i]) > 1e-5)
        {
            std::cerr << "Result verification failed at element " << i << std::endl;
            break;
        }
    }
}


// Function to perform some CPU-intensive work
void doCPUWork() {
    volatile int sum = 0;
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
}

// Function to write random data to a file
void writeToFile(const std::string& filename, int sizeMB) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }

    const int chunkSize = 1024 * 1024; // 1 MB
    std::vector<char> buffer(chunkSize);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    for (int i = 0; i < sizeMB; ++i) {
        for (int j = 0; j < chunkSize; ++j) {
            buffer[j] = static_cast<char>(dis(gen));
        }
        file.write(buffer.data(), chunkSize);

        // Introduce a small delay to make the I/O activity more visible
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    file.close();
}

// Function to read data from a file
void readFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return;
    }

    const int chunkSize = 1024 * 1024; // 1 MB
    std::vector<char> buffer(chunkSize);

    while (file) {
        file.read(buffer.data(), chunkSize);
        // Introduce a small delay to make the I/O activity more visible
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    file.close();
}

int main() {
    std::vector<std::shared_ptr<IDevice>> devices;

    auto* device = new IOFile();
    devices.emplace_back((IDevice*)device);
    auto* device2 = new CPUPerf();
    devices.emplace_back((IDevice*)device2);
    auto* gpuDevice = new GPUFile();
    devices.emplace_back((IDevice*)gpuDevice);

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