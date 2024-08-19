#include "GPUFile.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "fmt/format.h"

static const std::vector<Metric> METRICS{
    Metric(POLLING, "raw_batches_n", true),
    Metric(POLLING, "raw_batches_ok", true),
    Metric(POLLING, "raw_batches_err", true),
    Metric(POLLING, "raw_batches_avg_submit_latency", true),
    Metric(POLLING, "raw_reads_n", true),
    Metric(POLLING, "raw_reads_ok", true),
    Metric(POLLING, "raw_reads_err", true),
    Metric(POLLING, "raw_reads_MiB", true),
    Metric(POLLING, "raw_reads_bandwidth", true),
    Metric(POLLING, "raw_reads_avg_latency", true),
    Metric(POLLING, "raw_writes_n", true),
    Metric(POLLING, "raw_writes_ok", true),
    Metric(POLLING, "raw_writes_err", true),
    Metric(POLLING, "raw_writes_MiB", true),
    Metric(POLLING, "raw_writes_bandwidth", true),
    Metric(POLLING, "raw_writes_avg_latency", true),
    Metric(CALCULATED, "batches_n", false),
    Metric(CALCULATED, "batches_ok", false),
    Metric(CALCULATED, "batches_err", false),
    Metric(CALCULATED, "reads_n", false),
    Metric(CALCULATED, "reads_ok", false),
    Metric(CALCULATED, "reads_err", false),
    Metric(CALCULATED, "reads_MiB", false),
    Metric(CALCULATED, "writes_n", false),
    Metric(CALCULATED, "writes_ok", false),
    Metric(CALCULATED, "writes_err", false),
    Metric(CALCULATED, "writes_MiB", false)
};

GPUFile::GPUFile() : Device(METRICS) {}

void GPUFile::readGPUStats() {
    std::ifstream file("/proc/driver/nvidia-fs/stats");
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error opening /proc/driver/nvidia-fs/stats" << std::endl;
        return;
    }

    bool startReading = false;
    while (std::getline(file, line)) {
        if (line.find("Batches") != std::string::npos) {
            startReading = true;
        }
        
        if (startReading) {
            std::istringstream iss(line);
            std::string key, value;
            
            iss >> key;
            if (key == "Batches" || key == "Reads" || key == "Writes") {
                std::string temp;
                uint64_t n, ok, err;
                iss >> temp >> n >> temp >> ok >> temp >> err;
                
                currentValues[key + "_n"] = n;
                currentValues[key + "_ok"] = ok;
                currentValues[key + "_err"] = err;
                
                if (key == "Batches") {
                    double avg_submit_latency;
                    iss >> temp >> avg_submit_latency;
                    currentValues[key + "_avg_submit_latency"] = static_cast<uint64_t>(avg_submit_latency * 1000); // Convert to microseconds
                }
                
                if (key == "Reads" || key == "Writes") {
                    uint64_t mib;
                    iss >> temp >> mib;
                    currentValues[key + "_MiB"] = mib;
                    
                    // Read the next line for bandwidth and latency
                    std::getline(file, line);
                    iss.clear();
                    iss.str(line);
                    
                    double bandwidth, avg_latency;
                    iss >> temp >> temp >> bandwidth >> temp >> temp >> avg_latency;
                    currentValues[key + "_bandwidth"] = static_cast<uint64_t>(bandwidth);
                    currentValues[key + "_avg_latency"] = static_cast<uint64_t>(avg_latency);
                }
            }
        }
    }
}

std::vector<std::pair<Metric, Measurement>> GPUFile::getData(SamplingMethod sampler) {
    readGPUStats();
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

Measurement GPUFile::fetchMetric(const Metric& metric) {
    std::string metricName = metric.name.substr(4); // Remove "raw_" prefix
    auto it = currentValues.find(metricName);
    if (it != currentValues.end()) {
        return Measurement(std::to_string(it->second));
    }
    return Measurement("0");
}

Measurement GPUFile::calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>>& requestedMetricsByDeviceBySamplingMethod) {
    std::string rawMetricName = "raw_" + metric.name;
    uint64_t prevValue = 0, currentValue = 0;

    auto deviceData = requestedMetricsByDeviceBySamplingMethod.find(getDeviceName());
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

std::string GPUFile::getDeviceName() {
    return "GPUFile";
}

std::unordered_map<std::string, Metric> GPUFile::getAllDeviceMetricsByName() {
    std::unordered_map<std::string, Metric> result;
    for (const auto& metric : METRICS) {
        result.emplace(metric.name, metric);
    }
    return result;
}

std::unordered_map<std::string, std::vector<Metric>> GPUFile::getNeededMetricsForCalculatedMetrics(const Metric& metric) {
    const size_t metricIndex = std::distance(METRICS.begin(), std::find(METRICS.begin(), METRICS.end(), metric));
    return {{getDeviceName(), {METRICS[metricIndex - METRICS.size() / 2]}}};
}