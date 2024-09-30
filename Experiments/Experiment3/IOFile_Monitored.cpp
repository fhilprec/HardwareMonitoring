#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "Device.hpp"
#include "IOFileTwoShot.h"
#include "Monitor.h"




// Function to perform some CPU-intensive work
void doCPUWork() {
    volatile int sum = 0;
    for (int i = 0; i < 10000000; ++i) {
        sum += i * i;
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

    auto* device2 = new IOFileTwoShot();
    devices.emplace_back((IDevice*)device2);

    std::filesystem::path outputDirectory("testOutput");
    auto fullPath = absolute(outputDirectory);

    // Create the Monitor object with the correct constructor signature
    Monitor monitor({devices,std::chrono::milliseconds(500),outputDirectory, outputDirectory});

    // Note: We can't set the polling time here as the constructor doesn't accept it.
    // If you need to set a custom polling time, you might need to modify the Monitor class.

    monitor.start();

    const std::string filename = "test_file.bin";
    const int fileSizeMB = 50; // 50 MB file

    std::cout << "Writing to file..." << std::endl;
    writeToFile(filename, fileSizeMB);

    std::cout << "Reading from file..." << std::endl;
    readFromFile(filename);


    monitor.stop();



    return 0;
}