#include "GPUComputation.h"
#include <iostream>
#include <sstream>
#include <memory>
#include <array>
#include "fmt/format.h"

static const std::vector<Metric> METRICS{
    Metric(POLLING, "gpu_temperature", true),
    Metric(POLLING, "gpu_power_usage", true),
    Metric(POLLING, "gpu_memory_used", true),
    Metric(POLLING, "gpu_memory_total", true),
    Metric(POLLING, "gpu_utilization", true)
};

GPUComputation::GPUComputation() : Device(METRICS) {}

void GPUComputation::readGPUStats() {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("nvidia-smi --query-gpu=temperature.gpu,power.draw,memory.used,memory.total,utilization.gpu --format=csv,noheader,nounits", "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    std::istringstream iss(result);
    std::string line;
    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string value;
        int i = 0;
        while (std::getline(lineStream, value, ',')) {
            switch (i) {
                case 0: currentValues["gpu_temperature"] = std::stoul(value); break;
                case 1: currentValues["gpu_power_usage"] = std::stoul(value); break;
                case 2: currentValues["gpu_memory_used"] = std::stoul(value); break;
                case 3: currentValues["gpu_memory_total"] = std::stoul(value); break;
                case 4: currentValues["gpu_utilization"] = std::stoul(value); break;
            }
            i++;
        }
    }
}

std::vector<std::pair<Metric, Measurement>> GPUComputation::getData(SamplingMethod sampler) {
    if (sampler == POLLING) readGPUStats();
    return Device::getData(sampler);
}

Measurement GPUComputation::fetchMetric(const Metric& metric) {
    auto it = currentValues.find(metric.name);
    if (it != currentValues.end()) {
        return Measurement(std::to_string(it->second));
    }
    return Measurement("0");
}

Measurement GPUComputation::calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<
                                    bool, std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                    requestedMetricsByDeviceBySamplingMethod, size_t timeIndexForMetric) {
    if (timeIndexForMetric == 0) return Measurement("0");
    
    auto deviceData = requestedMetricsByDeviceBySamplingMethod.at(getDeviceName()).at(POLLING).at(false);
    uint64_t prevValue = std::stoull(deviceData.at(timeIndexForMetric-1).at(metric).value);
    uint64_t currentValue = std::stoull(deviceData.at(timeIndexForMetric).at(metric).value);

    return Measurement(fmt::format("{}", currentValue - prevValue));
}

std::string GPUComputation::getDeviceName() {
    return "GPUComputation";
}

std::unordered_map<std::string, Metric> GPUComputation::getAllDeviceMetricsByName() {
    std::unordered_map<std::string, Metric> result;
    for (const auto& metric : METRICS) {
        result.emplace(metric.name, metric);
    }
    return result;
}

std::unordered_map<std::string, std::vector<Metric>> GPUComputation::getNeededMetricsForCalculatedMetrics(const Metric& metric) {
    return {{getDeviceName(), {metric}}};
}