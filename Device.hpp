#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>

#include "CalculateMetric.h"
#include "Measurement.h"
#include "Metric.h"

class Device
{
private:
    std::vector<Metric> allowedMetrics;
    std::vector<CalculateMetric> allowedCalculatedMetrics;
    std::string name;

protected:
    std::vector<Metric> userGivenPollingMetrics;
    std::vector<Metric> userGivenOneShotMetrics;
    std::vector<Metric> userGivenTwoShotMetrics;
    std::vector<CalculateMetric> userGivenCalculationMetrics;

    Device(const std::vector<Metric>& rawMetrics, const std::vector<CalculateMetric>& calculatedMetrics, const std::vector<Metric> &userMetrics, std::string  name);
    void initMetrics(const std::vector<Metric>& metricsToCount);

public:

    virtual ~Device() = default;
    Device() = default;
    virtual std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler);
    virtual Measurement fetchMetric(const Metric &metric) { return Measurement({}); }

public:
    std::vector<Metric> getAllowedMetrics() ;
    const std::vector<CalculateMetric> &getCalculatableMetrics() const;
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
