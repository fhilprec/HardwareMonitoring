#pragma once
#include <vector>
#include <unordered_map>
#include <filesystem>
#include "Device.hpp"

class NIC : public Device<NIC>
{
private:
    std::unordered_map<std::string, std::filesystem::path> counters;
    std::unordered_map<std::string, uint64_t> currentValues;
    std::string hwCountersPath;
    std::string portCountersPath;
    void readNICStats();
    void initCounters();

public:
    NIC(const std::string&, const int port);
    ~NIC() override = default;

    std::vector<std::pair<Metric, Measurement>> getData(SamplingMethod sampler) override;
    Measurement fetchMetric(const Metric& metric) override;
    Measurement calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<
                                bool, std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                requestedMetricsByDeviceBySamplingMethod, size_t timeIndexForMetric) override;

    static std::string getDeviceName();
    static std::unordered_map<std::string, Metric> getAllDeviceMetricsByName();
    static std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsForCalculatedMetrics(const Metric& metric);
};