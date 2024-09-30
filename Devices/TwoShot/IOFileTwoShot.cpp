#include "IOFileTwoShot.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "fmt/format.h"
#include "CPUPerfTwoShot.h"

static const std::vector<Metric> METRICS{
    Metric(TWO_SHOT, "rchar", true),
    Metric(TWO_SHOT, "wchar", true),
    Metric(TWO_SHOT, "syscr", true),
    Metric(TWO_SHOT, "syscw", true),
    Metric(TWO_SHOT, "read_bytes", true),
    Metric(TWO_SHOT, "write_bytes", true),
    Metric(TWO_SHOT, "cancelled_write_bytes", true),
};

IOFileTwoShot::IOFileTwoShot() : Device(METRICS) {}

void IOFileTwoShot::readIOStats() {
    std::ifstream file("/proc/self/io");
    std::string line;
    
    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Error opening file 'proc/self/io' for {} Device", getDeviceName()));
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        uint64_t value;

        if (iss >> key >> value) {
            key.pop_back(); // Remove the colon at the end of the key
            currentValues[key] = value;
        }
    }
}

std::vector<std::pair<Metric, Measurement>> IOFileTwoShot::getData(SamplingMethod sampler) {
    if(sampler == TWO_SHOT) readIOStats();
    auto result = Device::getData(sampler);
    return result;
}

Measurement IOFileTwoShot::fetchMetric(const Metric& metric) {
    auto it = currentValues.find(metric.name);
    if (it != currentValues.end()) {
        return Measurement(std::to_string(it->second));
    }
    return Measurement("0");
}

Measurement IOFileTwoShot::calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<
                                    bool, std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                    requestedMetricsByDeviceBySamplingMethod, size_t timeIndexForMetric) {
    if(metric.name == "rchar and cycles") {
        auto calcmetricvectorIOFile = requestedMetricsByDeviceBySamplingMethod.at(IOFileTwoShot::getDeviceName()).at(TWO_SHOT).at(false).at(1);
        auto calcmetricvectorPerf = requestedMetricsByDeviceBySamplingMethod.at(CPUPerfTwoShot::getDeviceName()).at(TWO_SHOT).at(false).at(1);

        int rchar = std::stoi(calcmetricvectorIOFile.at(IOFileTwoShot::getAllDeviceMetricsByName().at("rchar")).value);
        double cycles = std::stod(calcmetricvectorPerf.at(CPUPerfTwoShot::getAllDeviceMetricsByName().at("cycles")).value);

        return Measurement(fmt::format("{}", cycles/rchar));
    }

    if(timeIndexForMetric == 0) return Measurement("0");
    auto deviceData = requestedMetricsByDeviceBySamplingMethod.at(getDeviceName()).at(TWO_SHOT).at(false);
    uint64_t prevValue = std::stoull(deviceData.at(0).at(metric).value);
    uint64_t currentValue = std::stoull(deviceData.at(1).at(metric).value);

    return Measurement(fmt::format("{}", currentValue - prevValue));
}

std::string IOFileTwoShot::getDeviceName() {
    return "IOFileTwoShot";
}

std::unordered_map<std::string, Metric> IOFileTwoShot::getAllDeviceMetricsByName() {
    std::unordered_map<std::string, Metric> result;
    for (const auto& metric : METRICS) {
        result.emplace(metric.name, metric);
    }
    return result;
}

std::unordered_map<std::string, std::vector<Metric>> IOFileTwoShot::getNeededMetricsForCalculatedMetrics(const Metric& metric) {
    const size_t metricIndex = std::distance(METRICS.begin(), std::find(METRICS.begin(), METRICS.end(), metric));
    if (metric.name == "rchar and cycles") {
        return {{getDeviceName(), {getAllDeviceMetricsByName().at("rchar")}},
                {CPUPerfTwoShot::getDeviceName(), {CPUPerfTwoShot::getAllDeviceMetricsByName().at("cycles")}}};
    }
    return {{getDeviceName(), {METRICS[metricIndex]}}};
}