#include "IOFile.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "fmt/format.h"
#include "CPUPerf.h"


static const std::vector<Metric> METRICS{
    Metric(POLLING, "rchar", true),
    Metric(POLLING, "wchar", true),
    Metric(POLLING, "syscr", true),
    Metric(POLLING, "syscw", true),
    Metric(POLLING, "read_bytes", true),
    Metric(POLLING, "write_bytes", true),
    Metric(POLLING, "cancelled_write_bytes", true),
    Metric(POLLING, "rchar and cycles", false, true ,true)
};

IOFile::IOFile() : Device(METRICS) {}

void IOFile::readIOStats() {
    std::ifstream file("/proc/self/io");
    std::string line;
    
    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("Error opening file 'proc/self/io' for {} Device",getDeviceName()));
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

std::vector<std::pair<Metric, Measurement>> IOFile::getData(SamplingMethod sampler) {
    if(sampler == POLLING) readIOStats();
    auto result = Device::getData(sampler);
    return result;
}

Measurement IOFile::fetchMetric(const Metric& metric) {
    auto it = currentValues.find(metric.name);
    if (it != currentValues.end()) {
        return Measurement(std::to_string(it->second));
    }
    return Measurement("0");
}

Measurement IOFile::calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<
                                    bool, std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                    requestedMetricsByDeviceBySamplingMethod, size_t timeIndexForMetric) {
    uint64_t prevValue = 0, currentValue = 0;

    if(metric.name == "rchar and cycles") {
        auto calcmetricvectorIOFile = requestedMetricsByDeviceBySamplingMethod.at(IOFile::getDeviceName()).at(POLLING).at(timeIndexForMetric).at(true);
        auto calcmetricvectorPerf = requestedMetricsByDeviceBySamplingMethod.at(CPUPerf::getDeviceName()).at(POLLING).at(timeIndexForMetric).at(true);

        int rchar = std::stoi(calcmetricvectorIOFile[IOFile::getAllDeviceMetricsByName()["rchar"]].value);
        double cycles = std::stod(calcmetricvectorPerf[CPUPerf::getAllDeviceMetricsByName()["cycles"]].value);

        return Measurement(fmt::format("{}",cycles/rchar));
    }

    if(timeIndexForMetric == 0) return Measurement("0");
    auto deviceData = requestedMetricsByDeviceBySamplingMethod.at(getDeviceName()).at(POLLING).at(false);
    prevValue = std::stoull(deviceData.at(timeIndexForMetric-1).at(metric).value);
    currentValue = std::stoull(deviceData.at(timeIndexForMetric).at(metric).value);

    return Measurement(fmt::format("{}",currentValue - prevValue));
}

std::string IOFile::getDeviceName() {
    return "IOFile";
}

std::unordered_map<std::string, Metric> IOFile::getAllDeviceMetricsByName() {
    std::unordered_map<std::string, Metric> result;
    for (const auto& metric : METRICS) {
        result.emplace(metric.name, metric);
    }
    return result;
}

std::unordered_map<std::string, std::vector<Metric>> IOFile::getNeededMetricsForCalculatedMetrics(const Metric& metric) {
    const size_t metricIndex = std::distance(METRICS.begin(), std::find(METRICS.begin(),METRICS.end(), metric));
    if (metric.name == "rchar and cycles") {
        return {{getDeviceName(), {getAllDeviceMetricsByName()["rchar"]}},
            {CPUPerf::getDeviceName(), {CPUPerf::getAllDeviceMetricsByName()["cycles"]}}};
    }
    return {{getDeviceName(), {METRICS[metricIndex]}}};
}