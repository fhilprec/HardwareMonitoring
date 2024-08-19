// GPUFile.h
#pragma once
#include <vector>
#include <unordered_map>
#include "Device.hpp"

class GPUFile : public Device<GPUFile>
{
private:
    std::unordered_map<std::string, uint64_t> prevValues;
    std::unordered_map<std::string, uint64_t> currentValues;
    bool first = true;

    void readGPUStats();

public:
    GPUFile();
    ~GPUFile() override = default;

    std::vector<std::pair<Metric, Measurement>> getData(SamplingMethod sampler) override;
    Measurement fetchMetric(const Metric& metric) override;
    Measurement calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>>& requestedMetricsByDeviceBySamplingMethod) override;

    static std::string getDeviceName();
    static std::unordered_map<std::string, Metric> getAllDeviceMetricsByName();
    static std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsForCalculatedMetrics(const Metric& metric);
};
