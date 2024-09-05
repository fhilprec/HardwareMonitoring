#include "NIC.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include "fmt/format.h"

const std::vector<Metric> NIC::METRICS = {
    Metric(POLLING, "port_rcv_data", true, true, false),
    Metric(POLLING, "port_xmit_data", true, true, false),
    Metric(POLLING, "port_rcv_packets", true, true, false),
    Metric(POLLING, "port_xmit_packets", true, true, false),
    Metric(POLLING, "recv_rate", false, false, true),
    Metric(POLLING, "xmit_rate", false, false, true)
};

NIC::NIC(const std::string& nic, const std::string& port) 
    : Device(METRICS), nicName(nic), portNumber(port) {}

void NIC::readNICStats() {
    const std::string basePath = fmt::format("/sys/class/infiniband/{}/ports/{}/counters/", nicName, portNumber);
    
    for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
        const auto& path = entry.path();
        if (std::filesystem::is_regular_file(path)) {
            std::ifstream file(path);
            if (file.is_open()) {
                std::string value;
                if (std::getline(file, value)) {
                    currentValues[path.filename()] = std::stoull(value);
                }
            }
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

Measurement NIC::calculateMetric(const Metric& metric,
                                 const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<bool, std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                 requestedMetricsByDeviceBySamplingMethod,
                                 size_t timeIndexForMetric) {
    if (metric.name == "recv_rate" || metric.name == "xmit_rate") {
        if (timeIndexForMetric == 0) return Measurement("0");

        auto deviceData = requestedMetricsByDeviceBySamplingMethod.at(getDeviceName()).at(POLLING).at(false);
        std::string baseMetricName = metric.name == "recv_rate" ? "port_rcv_data" : "port_xmit_data";

        uint64_t prevValue = std::stoull(deviceData.at(timeIndexForMetric - 1).at(getAllDeviceMetricsByName()[baseMetricName]).value);
        uint64_t currentValue = std::stoull(deviceData.at(timeIndexForMetric).at(getAllDeviceMetricsByName()[baseMetricName]).value);

        // Assuming 1-second interval between polls, calculate bytes per second
        return Measurement(fmt::format("{}", currentValue - prevValue));
    }

    return Measurement("0");
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
    if (metric.name == "recv_rate") {
        return {{getDeviceName(), {getAllDeviceMetricsByName()["port_rcv_data"]}}};
    } else if (metric.name == "xmit_rate") {
        return {{getDeviceName(), {getAllDeviceMetricsByName()["port_xmit_data"]}}};
    }
    return {};
}