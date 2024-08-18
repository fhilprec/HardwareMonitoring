#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <chrono>
#include <functional>
#include <iomanip>
#include <string>

#include "Measurement.h"
#include "Metric.h"


class IDevice{
public:
    typedef std::function<std::unordered_map<std::string,Metric>()> getAllMetricsByNameType;
    typedef std::function<std::string()> getNameType;
    typedef std::function<std::unordered_map<std::string, std::vector<Metric>>(const Metric&)> getNeededMetricsOfOtherDevicesForCalculatedMetrics;

    [[nodiscard]] virtual std::string getName() const = 0;
    [[nodiscard]] virtual std::unordered_map<std::string,Metric> getAllMetricsByName() const=0;
    [[nodiscard]] virtual std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsOfOtherDevicesForCalculatedMetric(const Metric& calculatedMetric)=0;

    virtual std::vector<Metric> getUserMetrics() const = 0;
    virtual std::vector<std::pair<Metric, Measurement>> getData(SamplingMethod sampler) = 0;
    virtual Measurement fetchMetric(const Metric &metric) = 0;
    virtual Measurement calculateMetric(const Metric &metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::unordered_map<Metric, Measurement>>>> &requestedMetricsByDeviceBySamplingMethod) = 0;

    virtual ~IDevice()=default;

public:
    friend bool operator==(const IDevice& lhs, const IDevice& rhs)
    {
        return lhs.getName() == rhs.getName();
    }

    friend bool operator!=(const IDevice& lhs, const IDevice& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::size_t hash_value(const IDevice& obj)
    {
        return std::hash<std::string>{}(obj.getName());
    }
};

template<typename ActualDevice>
class Device : IDevice
{
private:
    std::vector<Metric> allowedMetrics;
    std::vector<Metric> userMetrics;
    std::string name;

    void initMetrics(const std::vector<Metric>& metricsToCount){
        std::vector<Metric> metricsToUse;
            for (const auto &metric: metricsToCount){
                if(ActualDevice::getAllDeviceMetricsByName().contains(metric.name)){
                    metricsToUse.push_back(metric);
                }
            }
            for (const auto &[name,metric]: ActualDevice::getAllDeviceMetricsByName()){
                if(metric.raw && std::find(metricsToUse.begin(), metricsToUse.end(),metric) == std::end(metricsToUse)){
                    metricsToUse.push_back(metric);
                }
            }

        for (const auto &metric: metricsToUse){
            switch (metric.samplingMethod)
            {
            case ONE_SHOT:
                userGivenOneShotMetrics.push_back(metric);
                break;
            case TWO_SHOT:
                userGivenTwoShotMetrics.push_back(metric);
                break;
            case POLLING:
                userGivenPollingMetrics.push_back(metric);
                break;
            case CALCULATED:
                userGivenCalculationMetrics.push_back(metric);
            }
        }
    };

protected:
    std::vector<Metric> userGivenPollingMetrics;
    std::vector<Metric> userGivenOneShotMetrics;
    std::vector<Metric> userGivenTwoShotMetrics;
    std::vector<Metric> userGivenCalculationMetrics;

    explicit Device(const std::vector<Metric> &userMetrics):userMetrics(userMetrics){
        static_assert(std::is_base_of_v<Device, ActualDevice>, "Template class should be the derived class of Device itself");
        static_assert(std::is_convertible_v<decltype(ActualDevice::getAllDeviceMetricsByName),IDevice::getAllMetricsByNameType>, "Should implement 'static std::unordered_map<std::string,Metric> getAllMetricsByName()'");
        static_assert(std::is_convertible_v<decltype(ActualDevice::getDeviceName),IDevice::getNameType>, "Should implement 'static std::string getName()'");
        static_assert(std::is_convertible_v<decltype(ActualDevice::getNeededMetricsForCalculatedMetrics),IDevice::getNeededMetricsOfOtherDevicesForCalculatedMetrics>, "Should implement 'static std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsForCalculatedMetrics(const Metric& metric)'");
        if(userMetrics.empty())
        {
            this->userMetrics = std::vector<Metric>();
            for (const auto &[name,metric]: ActualDevice::getAllDeviceMetricsByName()){
                this->userMetrics.push_back(metric);
            }
        }
        initMetrics(this->userMetrics);
    };
public:
    std::vector<Metric> getUserMetrics() const override {
        return userMetrics;
    };
    std::vector<std::pair<Metric, Measurement>> getData(SamplingMethod sampler) override {
        std::vector<std::pair<Metric, Measurement>> result;
        switch (sampler) {
        case POLLING:
            for (const auto &pollingMetric: userGivenPollingMetrics) {
                result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
            }
            break;
        case ONE_SHOT:
            for (const auto &pollingMetric: userGivenOneShotMetrics) {
                result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
            }
            break;
        case TWO_SHOT:
            for (const auto &pollingMetric: userGivenTwoShotMetrics) {
                result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
            }
            break;
        case CALCULATED:
            throw std::invalid_argument(
                    "This is not the intended way to get calculated Metrics, calculated metrics are registered with the device but only calculated after fetching the (raw) metrics");
        }
        return result;
    };

public:

    [[nodiscard]] std::string getName() const override {
        return ActualDevice::getDeviceName();
    }

    [[nodiscard]] std::unordered_map<std::string, Metric> getAllMetricsByName() const override {
        return ActualDevice::getAllDeviceMetricsByName();
    }

   [[nodiscard]] std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsOfOtherDevicesForCalculatedMetric(const Metric& calculatedMetric) override
    {
        return ActualDevice::getNeededMetricsForCalculatedMetrics(calculatedMetric);
    }

    ~Device() override = default;

};

