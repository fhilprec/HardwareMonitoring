#pragma once

#include <filesystem>
#include <iostream>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <vector>

#include "Device.hpp"
#include "Output.hpp"


struct Metric;
struct Measurement;

class FileManager {
    std::unordered_map<std::shared_ptr<IDevice>, std::filesystem::path> filePathsForDevices;
    std::unordered_map<std::shared_ptr<IDevice>, std::shared_ptr<std::ofstream>> tempDataForDevices;
    std::filesystem::path tempOutputDirectory;
    std::filesystem::path outputDirectory;

public:
    FileManager(const std::vector<std::shared_ptr<IDevice>> &devices,
                const std::filesystem::path& tempOutputDirectory,
                const std::filesystem::path& outputDirectory);
    void writeToBuffer(const std::shared_ptr<IDevice>& device, const std::vector<std::pair<Metric, Measurement>>& line);
    std::vector<std::unordered_map<Metric, Measurement>>  readAllFromBuffer(const std::shared_ptr<IDevice>& device) const;
    void save(std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<SamplingMethod, std::unordered_map<bool, std::vector
        <std::unordered_map<Metric, Measurement>>>>> data);
    static std::string createHeaderString(const std::vector<Metric>& metrics);

private:
    static std::string lineToString(const std::vector<std::pair<Metric, Measurement>>& line);
    static std::string lineToStringOrdered(const std::unordered_map<Metric, Measurement>& line, const std::vector<Metric>& metricsOrdered);
    static std::unordered_map<Metric, Measurement> stringToLine(const std::string& line, const std::vector<Metric>& readMetrics);
    static std::vector<Metric> readMetrics(const std::shared_ptr<IDevice>& device, const std::string& line);
};


