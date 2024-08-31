#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Device.hpp"

class NIC : public Device<NIC> {
private:
    std::string nicName;
    int port;
    double interval;
    std::string hwCountersPath;
    std::string portCountersPath;
    std::unordered_map<std::string, std::filesystem::path> counters;
    std::unordered_map<std::string, uint64_t> lastValues;
    
    static bool endsWith(const std::string& str, const std::string& suffix);
    void loadCounters();
    std::unordered_map<std::string, uint64_t> readCounters();

public:
    NIC(const std::string& nicName, int port);
    ~NIC() override = default;

    std::vector<std::pair<Metric, Measurement>> getData(SamplingMethod sampler) override;
    Measurement fetchMetric(const Metric& metric) override;
    Measurement calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>>& requestedMetricsByDeviceBySamplingMethod) override;

    static std::string getDeviceName();
    static std::unordered_map<std::string, Metric> getAllDeviceMetricsByName();
    static std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsForCalculatedMetrics(const Metric& metric);
};
