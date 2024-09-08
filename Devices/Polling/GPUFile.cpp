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
        throw std::runtime_error(fmt::format("Error opening file '{}' for {} Device", filename, getDeviceName()));
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
            if (sscanf(line.c_str(), "Batches                              : n=%lu ok=%lu err=%lu Avg-Submit-Latency(usec)=%lf",
                       &n, &ok, &err, &avg_latency) == 4) {
                currentValues["raw_batches_n"] = n;
                currentValues["raw_batches_ok"] = ok;
                currentValues["raw_batches_err"] = err;
                currentValues["raw_batches_avg_submit_latency"] = static_cast<uint64_t>(avg_latency);
            }
        } else if (key == "Reads") {
            uint64_t n, ok, err, readMiB, io_state_err;
            if (sscanf(line.c_str(), "Reads                                : n=%lu ok=%lu err=%lu readMiB=%lu io_state_err=%lu",
                       &n, &ok, &err, &readMiB, &io_state_err) == 5) {
                currentValues["raw_reads_n"] = n;
                currentValues["raw_reads_ok"] = ok;
                currentValues["raw_reads_err"] = err;
                currentValues["raw_reads_MiB"] = readMiB;
            }
            if (std::getline(file, line)) {
                double bandwidth, avg_latency;
                if (sscanf(line.c_str(), "Reads                                : Bandwidth(MiB/s)=%lf Avg-Latency(usec)=%lf",
                           &bandwidth, &avg_latency) == 2) {
                    currentValues["raw_reads_bandwidth"] = static_cast<uint64_t>(bandwidth);
                    currentValues["raw_reads_avg_latency"] = static_cast<uint64_t>(avg_latency);
                }
            }
        } else if (key == "Writes") {
            uint64_t n, ok, err, writeMiB, io_state_err, pg_cache, pg_cache_fail, pg_cache_eio;
            if (sscanf(line.c_str(), "Writes                               : n=%lu ok=%lu err=%lu writeMiB=%lu io_state_err=%lu pg-cache=%lu pg-cache-fail=%lu pg-cache-eio=%lu",
                       &n, &ok, &err, &writeMiB, &io_state_err, &pg_cache, &pg_cache_fail, &pg_cache_eio) == 8) {
                currentValues["raw_writes_n"] = n;
                currentValues["raw_writes_ok"] = ok;
                currentValues["raw_writes_err"] = err;
                currentValues["raw_writes_MiB"] = writeMiB;
            }
            if (std::getline(file, line)) {
                double bandwidth, avg_latency;
                if (sscanf(line.c_str(), "Writes                               : Bandwidth(MiB/s)=%lf Avg-Latency(usec)=%lf",
                           &bandwidth, &avg_latency) == 2) {
                    currentValues["raw_writes_bandwidth"] = static_cast<uint64_t>(bandwidth);
                    currentValues["raw_writes_avg_latency"] = static_cast<uint64_t>(avg_latency);
                }
            }
        }
    }
}

std::vector<std::pair<Metric, Measurement>> GPUFile::getData(SamplingMethod sampler) {
    if (sampler == POLLING) readGPUStats();
    return Device::getData(sampler);
}

Measurement GPUFile::fetchMetric(const Metric& metric) {
    auto it = currentValues.find(metric.name);
    if (it != currentValues.end()) {
        return Measurement(std::to_string(it->second));
    }
    return Measurement("0");
}

Measurement GPUFile::calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<
                                    bool, std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                    requestedMetricsByDeviceBySamplingMethod, size_t timeIndexForMetric) {
    if (timeIndexForMetric == 0) return Measurement("0");
    
    auto deviceData = requestedMetricsByDeviceBySamplingMethod.at(getDeviceName()).at(POLLING).at(false);
    uint64_t prevValue = std::stoull(deviceData.at(timeIndexForMetric-1).at(metric).value);
    uint64_t currentValue = std::stoull(deviceData.at(timeIndexForMetric).at(metric).value);

    return Measurement(fmt::format("{}", currentValue - prevValue));
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
    return {{getDeviceName(), {metric}}};
}