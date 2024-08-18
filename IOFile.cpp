#include "IOFile.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "fmt/format.h"
#include "CPUPerf.h"


static const std::vector<Metric> METRICS{
    Metric(TWO_SHOT, "raw_rchar", true),
    Metric(TWO_SHOT, "raw_wchar", true),
    Metric(TWO_SHOT, "raw_syscr", true),
    Metric(TWO_SHOT, "raw_syscw", true),
    Metric(TWO_SHOT, "raw_read_bytes", true),
    Metric(TWO_SHOT, "raw_write_bytes", true),
    Metric(TWO_SHOT, "raw_cancelled_write_bytes", true),
    Metric(CALCULATED, "rchar", false),
    Metric(CALCULATED, "wchar", false),
    Metric(CALCULATED, "syscr", false),
    Metric(CALCULATED, "syscw", false),
    Metric(CALCULATED, "read_bytes", false),
    Metric(CALCULATED, "write_bytes", false),
    Metric(CALCULATED, "cancelled_write_bytes", false),
    Metric(CALCULATED, "rchar and cycles", false)
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
    readIOStats();
    auto result = Device::getData(sampler);
    if (sampler == TWO_SHOT) {
        if (first) {
            first = false;
        } else {
            prevValues = currentValues;
        }
    }
    return result;
}

Measurement IOFile::fetchMetric(const Metric& metric) {
    std::string metricName = metric.name.substr(4); // Remove "raw_" prefix
    auto it = currentValues.find(metricName);
    if (it != currentValues.end()) {
        return Measurement(std::to_string(it->second));
    }
    return Measurement("0");
}

Measurement IOFile::calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::unordered_map<Metric, Measurement>>>> &requestedMetricsByDeviceBySamplingMethod) {
    std::string rawMetricName = "raw_" + metric.name;
    uint64_t prevValue = 0, currentValue = 0;

    auto deviceData = requestedMetricsByDeviceBySamplingMethod.find(getDeviceName());

    if(metric.name == "rchar and cycles") {
        auto calcmetricvectorIOFile = requestedMetricsByDeviceBySamplingMethod.at(IOFile::getDeviceName()).at(CALCULATED)[0];
        auto calcmetricvectorPerf = requestedMetricsByDeviceBySamplingMethod.at(CPUPerf::getDeviceName()).at(CALCULATED)[0];

        int rchar = std::stoi(calcmetricvectorIOFile[IOFile::getAllDeviceMetricsByName()["rchar"]].value);
        double cycles = std::stod(calcmetricvectorPerf[CPUPerf::getAllDeviceMetricsByName()["cycles"]].value);

        return Measurement(fmt::format("{}",cycles/rchar));
    }


    if (deviceData == requestedMetricsByDeviceBySamplingMethod.end()) {
        return Measurement("0");
    }

    auto samplingData = deviceData->second.find(TWO_SHOT);
    if (samplingData == deviceData->second.end() || samplingData->second.size() < 2) {
        return Measurement("0");
    }

    for (const auto& [m, measurement] : samplingData->second[0]) {
        if (m.name == rawMetricName) {
            prevValue = std::stoull(measurement.value);
            break;
        }
    }

    for (const auto& [m, measurement] : samplingData->second[1]) {
        if (m.name == rawMetricName) {
            currentValue = std::stoull(measurement.value);
            break;
        }
    }

    return Measurement(std::to_string(currentValue - prevValue));
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
    return {{getDeviceName(), {METRICS[metricIndex - METRICS.size() / 2]}}};
}