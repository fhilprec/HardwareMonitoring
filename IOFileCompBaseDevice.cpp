#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "Device.hpp"
#include "IOFileTwoShot.h"
#include "Monitor.h"




// Function to read IO stats from /proc/self/io
std::unordered_map<std::string, uint64_t> readIOStats() {
    std::ifstream file("/proc/self/io");
    std::string line;
    std::unordered_map<std::string, uint64_t> stats;
    
    if (!file.is_open()) {
        std::cerr << "Error opening file 'proc/self/io'" << std::endl;
        return stats;
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        uint64_t value;

        if (iss >> key >> value) {
            key.pop_back(); // Remove the colon at the end of the key
            stats[key] = value;
        }
    }

    return stats;
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
    const std::string filename = "test_file.bin";
    const int fileSizeMB = 50; // 50 MB file

    // Create testOutput directory if it doesn't exist
    std::filesystem::create_directories("testOutput");

    // Open output file
    std::ofstream outFile("testOutput/io_stats.csv");
    if (!outFile) {
        std::cerr << "Failed to open output file" << std::endl;
        return 1;
    }

    // Write header
    outFile << "rchar;wchar;syscr;syscw;read_bytes;write_bytes;cancelled_write_bytes" << std::endl;

    // First measurement
    auto startStats = readIOStats();

    std::cout << "Writing to file..." << std::endl;
    writeToFile(filename, fileSizeMB);

    std::cout << "Reading from file..." << std::endl;
    readFromFile(filename);

    // Second measurement
    auto endStats = readIOStats();

    // Calculate and write differences
    outFile << (endStats["rchar"] - startStats["rchar"]) << ";"
            << (endStats["wchar"] - startStats["wchar"]) << ";"
            << (endStats["syscr"] - startStats["syscr"]) << ";"
            << (endStats["syscw"] - startStats["syscw"]) << ";"
            << (endStats["read_bytes"] - startStats["read_bytes"]) << ";"
            << (endStats["write_bytes"] - startStats["write_bytes"]) << ";"
            << (endStats["cancelled_write_bytes"] - startStats["cancelled_write_bytes"]) << std::endl;

    outFile.close();

    std::cout << "IO stats have been written to testOutput/io_stats.csv" << std::endl;

    return 0;
}