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
    Metric(POLLING, "raw_writes_avg_latency", true)
};

GPUFile::GPUFile() : Device(METRICS) {}

void GPUFile::readGPUStats() {
    const char* filename = "/proc/driver/nvidia-fs/stats";
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error opening " << filename << std::endl;
        return;
    }

    // Reset all values to 0 before reading
    for (const auto& metric : METRICS) {
        currentValues[metric.name] = 0;
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "Batches") {
            uint64_t n, ok, err;
            double avg_latency;
            int result = sscanf(line.c_str(), "Batches                              : n=%lu ok=%lu err=%lu Avg-Submit-Latency(usec)=%lf",
                                &n, &ok, &err, &avg_latency);
            if (result == 4) {
                currentValues["raw_batches_n"] = n;
                currentValues["raw_batches_ok"] = ok;
                currentValues["raw_batches_err"] = err;
                currentValues["raw_batches_avg_submit_latency"] = static_cast<uint64_t>(avg_latency);
            }
        } else if (key == "Reads") {
            uint64_t n, ok, err, readMiB, io_state_err;
            int result = sscanf(line.c_str(), "Reads                                : n=%lu ok=%lu err=%lu readMiB=%lu io_state_err=%lu",
                                &n, &ok, &err, &readMiB, &io_state_err);
            if (result == 5) {
                currentValues["raw_reads_n"] = n;
                currentValues["raw_reads_ok"] = ok;
                currentValues["raw_reads_err"] = err;
                currentValues["raw_reads_MiB"] = readMiB;
            }
            // Read the next line for bandwidth and latency
            if (std::getline(file, line)) {
                double bandwidth, avg_latency;
                result = sscanf(line.c_str(), "Reads                                : Bandwidth(MiB/s)=%lf Avg-Latency(usec)=%lf",
                                &bandwidth, &avg_latency);
                if (result == 2) {
                    currentValues["raw_reads_bandwidth"] = static_cast<uint64_t>(bandwidth);
                    currentValues["raw_reads_avg_latency"] = static_cast<uint64_t>(avg_latency);
                }
            }
        } else if (key == "Writes") {
            uint64_t n, ok, err, writeMiB, io_state_err, pg_cache, pg_cache_fail, pg_cache_eio;
            int result = sscanf(line.c_str(), "Writes                               : n=%lu ok=%lu err=%lu writeMiB=%lu io_state_err=%lu pg-cache=%lu pg-cache-fail=%lu pg-cache-eio=%lu",
                                &n, &ok, &err, &writeMiB, &io_state_err, &pg_cache, &pg_cache_fail, &pg_cache_eio);
            if (result == 8) {
                currentValues["raw_writes_n"] = n;
                currentValues["raw_writes_ok"] = ok;
                currentValues["raw_writes_err"] = err;
                currentValues["raw_writes_MiB"] = writeMiB;
            }
            // Read the next line for bandwidth and latency
            if (std::getline(file, line)) {
                double bandwidth, avg_latency;
                result = sscanf(line.c_str(), "Writes                               : Bandwidth(MiB/s)=%lf Avg-Latency(usec)=%lf",
                                &bandwidth, &avg_latency);
                if (result == 2) {
                    currentValues["raw_writes_bandwidth"] = static_cast<uint64_t>(bandwidth);
                    currentValues["raw_writes_avg_latency"] = static_cast<uint64_t>(avg_latency);
                }
            }
        }
    }
}



std::vector<std::pair<Metric, Measurement>> GPUFile::getData(SamplingMethod sampler) {
    readGPUStats();
    std::vector<std::pair<Metric, Measurement>> result;
    result.reserve(METRICS.size());  // Pre-allocate memory to avoid reallocations

    for (const auto& metric : METRICS) {
        if (metric.samplingMethod == POLLING) {
            auto it = currentValues.find(metric.name);
            if (it != currentValues.end()) {
                result.emplace_back(metric, Measurement(std::to_string(it->second)));
            } else {
                result.emplace_back(metric, Measurement("0"));
            }
        } else if (sampler == TWO_SHOT && !first) {
            auto rawMetricName = "raw_" + metric.name;
            auto currentIt = currentValues.find(rawMetricName);
            auto prevIt = prevValues.find(rawMetricName);
            if (currentIt != currentValues.end() && prevIt != prevValues.end()) {
                uint64_t diff = currentIt->second - prevIt->second;
                result.emplace_back(metric, Measurement(std::to_string(diff)));
            } else {
                result.emplace_back(metric, Measurement("0"));
            }
        }
    }

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