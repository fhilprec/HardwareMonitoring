#include "NIC.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "fmt/format.h"

static const std::vector<Metric> METRICS{
    Metric(POLLING, "port_rcv_data", true),
    Metric(POLLING, "port_xmit_data", true),
    Metric(POLLING, "port_rcv_packets", true),
    Metric(POLLING, "port_xmit_packets", true)
};

NIC::NIC(const std::string& deviceName = "mlx5_0", const int portNumber = 1) : Device(METRICS) {
    hwCountersPath = fmt::format("/sys/class/infiniband/{}/ports/{}/hw_counters/", deviceName, portNumber);
    portCountersPath = fmt::format("/sys/class/infiniband/{}/ports/{}/counters/", deviceName, portNumber);
    initCounters();
}

void NIC::initCounters() {
    for (const auto& path : {hwCountersPath, portCountersPath}) {
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                if (std::filesystem::is_regular_file(entry.path())) {
                    counters[entry.path().filename().string()] = entry.path();
                }
            }
        }
}

void NIC::readNICStats() {
    for (const auto& [name, path] : counters) {
        std::ifstream file(path);
        if (file.is_open()) {
            uint64_t value;
            file >> value;
            currentValues[name] = value;
        }
    }
}

std::vector<std::pair<Metric, Measurement>> NIC::getData(SamplingMethod sampler) {
    if (sampler == POLLING) readNICStats();
    return Device::getData(sampler);
}

Measurement NIC::fetchMetric(const Metric& metric) {
    auto it = currentValues.find(metric.name);
    if (it != currentValues.end()) {
        return Measurement(std::to_string(it->second));
    }
    return Measurement("0");
}

Measurement NIC::calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<
                                bool, std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                requestedMetricsByDeviceBySamplingMethod, size_t timeIndexForMetric) {
    uint64_t prevValue = 0, currentValue = 0;

    if(timeIndexForMetric == 0) return Measurement("0");
    
    auto deviceData = requestedMetricsByDeviceBySamplingMethod.at(getDeviceName()).at(POLLING).at(false);
    prevValue = std::stoull(deviceData.at(timeIndexForMetric-1).at(metric).value);
    currentValue = std::stoull(deviceData.at(timeIndexForMetric).at(metric).value);

    return Measurement(fmt::format("{}", currentValue - prevValue));
}

std::string NIC::getDeviceName() {
    return "NIC";
}

std::unordered_map<std::string, Metric> NIC::getAllDeviceMetricsByName() {
    std::unordered_map<std::string, Metric> result;
    for (const auto& metric : METRICS) {
        result.emplace(metric.name, metric);
    }
    return result;
}

std::unordered_map<std::string, std::vector<Metric>> NIC::getNeededMetricsForCalculatedMetrics(const Metric& metric) {
    // For NIC metrics, we don't need any additional metrics for calculations
    return {{getDeviceName(), {metric}}};
}


static const std::vector METRICS{
    Metric(POLLING, "cycles", true),
    Metric(POLLING, "L1-misses", true)
};

Measurement NIC::fetchMetric(const Metric& metric) {
    auto result = myFunctionToFetchMetric(metric);
    return Measurement(result);
}

//Some additional helper functions like
std::unordered_map<std::string, Metric>
NIC::getAllDeviceMetricsByName();

std::unordered_map<std::string, std::vector<Metric>>
NIC::getNeededMetricsForCalculatedMetrics(const Metric& metric);