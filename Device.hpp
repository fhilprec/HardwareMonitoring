#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>

#include "Measurement.h"
#include "Metric.h"

#define LOG(x) std::cout << x << std::endl

class Device
{
    std::vector<Metric> allowedMetrics;
    std::string name;

protected:
    std::vector<Metric> pollingMetrics;
    std::vector<Metric> oneShotMetrics;
    std::vector<Metric> twoShotMetrics;

public:
    virtual ~Device() = default;
    Device() : Device(""){};
    explicit Device(std::string name):name(std::move(name)){};
    Device(std::string  name, const std::vector<Metric>& metrics);
    virtual std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler);
    virtual Measurement fetchMetric(const Metric &metric) { return Measurement({}); };

public:
    const std::vector<Metric> &getAllowedMetrics() const;
    const std::string &getName() const;

public:
    bool operator==(const Device &rhs) const;
    bool operator!=(const Device &rhs) const;
};

template <>
struct std::hash<Device>
{
    size_t operator()(const Device& device) const noexcept
    {
        return std::hash<std::string>{}(device.getName());
    }
};
