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


class IDevice{
public:
    typedef std::function<std::unordered_map<std::string,Metric>()> getAllMetricsByNameType;
    typedef std::function<std::string()> getNameType;

    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual std::unordered_map<std::string,Metric> getAllMetricsByName() const=0;

    virtual std::vector<Metric> getUserMetrics() const = 0;
    virtual std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler) = 0;
    virtual Measurement fetchMetric(const Metric &metric) = 0;

    IDevice() = default;
    virtual ~IDevice()= default;

    bool operator==(const IDevice &rhs) const;
};
template<typename ActualDevice>
class Device : IDevice
{
private:
    std::vector<Metric> allowedMetrics;
    std::vector<Metric> userMetrics;
    std::string name;

    void initMetrics(const std::vector<Metric>& metricsToCount);

protected:
    std::vector<Metric> userGivenPollingMetrics;
    std::vector<Metric> userGivenOneShotMetrics;
    std::vector<Metric> userGivenTwoShotMetrics;
    std::vector<Metric> userGivenCalculationMetrics;

    explicit Device(const std::vector<Metric> &userMetrics);
public:
    std::vector<Metric> getUserMetrics() const override;
    std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler) override;

public:

    [[nodiscard]] std::string getName() const override {
        return ActualDevice::getDeviceName();
    }

    [[nodiscard]] std::unordered_map<std::string, Metric> getAllMetricsByName() const override {
        return ActualDevice::getAllDeviceMetricsByName();
    }

public:

    virtual ~Device() = default;
    Device() = default;

};


template <typename T>
struct std::hash<Device<T>>
{
    size_t operator()(const Device<T>& device) const noexcept
    {
        return std::hash<std::string>{}(T::getName());
    }
};
