#pragma once

#include "Device.hpp"
#include <unordered_map>
#include <vector>
#include <string>

class NIC : public Device<NIC> {
private:
    std::unordered_map<std::string, uint64_t> currentValues;
    void readNICStats();

    static const std::vector<Metric> METRICS;
    std::string nicName;
    std::string portNumber;

public:
    NIC(const std::string& nic = "mlx5_0", const std::string& port = "1");
    ~NIC() override = default;

    std::vector<std::pair<Metric, Measurement>> getData(SamplingMethod sampler) override;
    Measurement fetchMetric(const Metric& metric) override;
    Measurement calculateMetric(const Metric& metric, 
                                const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<bool, std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                requestedMetricsByDeviceBySamplingMethod, 
                                size_t timeIndexForMetric) override;

    static std::string getDeviceName();
    static std::unordered_map<std::string, Metric> getAllDeviceMetricsByName();
    static std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsForCalculatedMetrics(const Metric& metric);

    // New methods to get and set NIC and port
    std::string getNicName() const { return nicName; }
    std::string getPortNumber() const { return portNumber; }
    void setNicName(const std::string& nic) { nicName = nic; }
    void setPortNumber(const std::string& port) { portNumber = port; }
};