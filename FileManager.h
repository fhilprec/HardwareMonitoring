#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "Device.hpp"
#include "Output.hpp"


struct Metric;
struct Measurement;

class FileManager {
    std::unordered_map<Device, std::filesystem::path> filePathsForDevices;
    std::unordered_map<Device, std::shared_ptr<std::ofstream>> tempDataForDevices;
    std::optional<std::filesystem::path> outputDirectory;

public:
    FileManager(const std::vector<std::shared_ptr<Device>>& devices, const std::optional<std::filesystem::path>& outputDirectory);
    void writeToBuffer(const Device& device, const std::vector<std::pair<Metric, Measurement>>& line);
    std::vector<std::vector<std::pair<Metric, Measurement>>> readAllFromBuffer(const Device &device) const;
    void save(std::unordered_map<Device, std::unordered_map<Sampler, std::vector<std::vector<std::pair<Metric, Measurement>>>>> data);
    static std::string createHeaderString(const std::vector<Metric>& metrics);

private:
    static std::string lineToString(const std::vector<std::pair<Metric, Measurement>>& line);
    std::string lineToStringOrdered(const std::vector<std::pair<Metric, Measurement>>& line,
                                    const std::vector<Metric>& metricsOrdered);
    static std::vector<std::pair<Metric, Measurement>> stringToLine(const std::string& line, const std::vector<Metric>& readMetrics);
    static std::vector<Metric> readMetrics(const Device& device, const std::string& line);
};


