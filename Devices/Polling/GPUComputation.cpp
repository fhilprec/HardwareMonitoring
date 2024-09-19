#include "GPUComputation.h"
#include <iostream>
#include <sstream>
#include <nvml.h>
#include "fmt/format.h"

static const std::vector<Metric> METRICS{
    Metric(POLLING, "gpu_temperature", true),
    Metric(POLLING, "gpu_power_usage", true),
    Metric(POLLING, "gpu_memory_used", true),
    Metric(POLLING, "gpu_memory_total", true),
    Metric(POLLING, "gpu_utilization", true)
};

GPUComputation::GPUComputation() : Device(METRICS) {
    nvmlReturn_t result = nvmlInit();
    if (result != NVML_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to initialize NVML: {}", nvmlErrorString(result)));
    }
}

GPUComputation::~GPUComputation() {
    nvmlReturn_t result = nvmlShutdown();
    if (result != NVML_SUCCESS) {
        std::cerr << fmt::format("Failed to shut down NVML: {}", nvmlErrorString(result)) << std::endl;
    }
}

void GPUComputation::readGPUStats() {
    nvmlDevice_t device;
    nvmlReturn_t result = nvmlDeviceGetHandleByIndex(1, &device);  // Get handle for the first GPU
    if (result != NVML_SUCCESS) {
        throw std::runtime_error(fmt::format("Failed to get device handle: {}", nvmlErrorString(result)));
    }

    // GPU temperature
    unsigned int temperature;
    result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);
    if (result == NVML_SUCCESS) {
        currentValues["gpu_temperature"] = temperature;
    } else {
        std::cerr << fmt::format("Failed to get GPU temperature: {}", nvmlErrorString(result)) << std::endl;
    }

    // GPU power usage
    unsigned int powerUsage;
    result = nvmlDeviceGetPowerUsage(device, &powerUsage);
    if (result == NVML_SUCCESS) {
        currentValues["gpu_power_usage"] = powerUsage / 1000;  // Convert milliwatts to watts
    } else {
        std::cerr << fmt::format("Failed to get GPU power usage: {}", nvmlErrorString(result)) << std::endl;
    }

    // GPU memory usage
    nvmlMemory_t memoryInfo;
    result = nvmlDeviceGetMemoryInfo(device, &memoryInfo);
    if (result == NVML_SUCCESS) {
        currentValues["gpu_memory_used"] = memoryInfo.used / (1024 * 1024);   // Convert to MB
        currentValues["gpu_memory_total"] = memoryInfo.total / (1024 * 1024);  // Convert to MB
    } else {
        std::cerr << fmt::format("Failed to get GPU memory info: {}", nvmlErrorString(result)) << std::endl;
    }

    // GPU utilization
    nvmlUtilization_t utilization;
    result = nvmlDeviceGetUtilizationRates(device, &utilization);
    if (result == NVML_SUCCESS) {
        currentValues["gpu_utilization"] = utilization.gpu;
    } else {
        std::cerr << fmt::format("Failed to get GPU utilization: {}", nvmlErrorString(result)) << std::endl;
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