#include "Device.hpp"

#include <utility>


Device::Device(const std::vector<Metric>& metrics, const std::vector<CalculateMetric>& calculatedMetrics,
               const std::vector<Metric>& userMetrics,
               std::string name) : allowedMetrics(metrics), allowedCalculatedMetrics(calculatedMetrics),
                                   name(std::move(name))
{
    for (const auto& allowedMetric : allowedMetrics)
    {
        if (allowedMetric.samplingMethod == CALCULATED) throw std::invalid_argument(
            std::format("Metric '%s' should not have type 'Calculated'", allowedMetric.name));
    }
    initMetrics(userMetrics);
}

void Device::initMetrics(const std::vector<Metric>& metricsToCount)
{
    if (metricsToCount.empty())
    {
        userGivenCalculationMetrics.insert(userGivenCalculationMetrics.begin(), allowedCalculatedMetrics.begin(),
                                           allowedCalculatedMetrics.end());
        for (const auto& allowedMetric : allowedMetrics)
        {
            switch (allowedMetric.samplingMethod)
            {
            case ONE_SHOT:
                userGivenOneShotMetrics.push_back(allowedMetric);
                break;
            case TWO_SHOT:
                userGivenTwoShotMetrics.push_back(allowedMetric);
                break;
            case POLLING:
                userGivenPollingMetrics.push_back(allowedMetric);
                break;
            default: throw std::logic_error(std::format("Unexpected Sampling Method for Metric '%s'", allowedMetric.name));
            }
        }
    }else
    {
        for (const auto& metric : metricsToCount)
        {
            if (std::ranges::find(allowedMetrics, metric) == allowedMetrics.end() && std::ranges::find(
                allowedCalculatedMetrics, metric,
                [](auto&& computeMetric) { return static_cast<Metric>(computeMetric); }) == allowedCalculatedMetrics.end())
            {
                continue;
            }

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
                userGivenCalculationMetrics.push_back(*std::ranges::find(allowedCalculatedMetrics, metric,
                                                                         [](auto&& computeMetric)
                                                                         {
                                                                             return static_cast<Metric>(computeMetric);
                                                                         }));
            }
        }
        for (const auto& allowedMetric : allowedMetrics)
        {
            if(allowedMetric.raw)
            {
                switch (allowedMetric.samplingMethod)
                {
                case ONE_SHOT:
                    userGivenOneShotMetrics.push_back(allowedMetric);
                    break;
                case TWO_SHOT:
                    userGivenTwoShotMetrics.push_back(allowedMetric);
                    break;
                case POLLING:
                    userGivenPollingMetrics.push_back(allowedMetric);
                    break;
                default: throw std::logic_error(std::format("Unexpected Sampling Method for Metric '%s'", allowedMetric.name));
                }
            }
        }
    }
}

std::vector<std::pair<Metric, Measurement>> Device::getData(const Sampler sampler)
{
    std::vector<std::pair<Metric, Measurement>> result;
    switch (sampler)
    {
    case POLLING:
        for (const auto& pollingMetric : userGivenPollingMetrics)
        {
            result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
        }
        break;
    case ONE_SHOT:
        for (const auto& pollingMetric : userGivenOneShotMetrics)
        {
            result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
        }
        break;
    case TWO_SHOT:
        for (const auto& pollingMetric : userGivenTwoShotMetrics)
        {
            result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
        }
        break;
    case CALCULATED:
        throw std::invalid_argument(
            "This is not the intended way to get calculcated Metrics, calculated metrics are registered with the device but only calculated after fetching the (raw) metrics");
    }
    return result;
}

std::vector<Metric> Device::getAllowedMetrics() const
{
    std::vector<Metric> allAllowedMetrics(allowedMetrics.size() + allowedCalculatedMetrics.size());
    for (const auto & allowedMetric : allowedMetrics)
    {
        if(!allowedMetric.raw)
        {
            allAllowedMetrics.push_back(allowedMetric);
        }
    }
    allAllowedMetrics.insert(allAllowedMetrics.end(), allowedCalculatedMetrics.begin(), allowedCalculatedMetrics.end());
    return allAllowedMetrics;
}

const std::vector<CalculateMetric>& Device::getCalculatableMetrics() const
{
    return userGivenCalculationMetrics;
}

std::vector<Metric> Device::getUserMetrics() const
{
    std::vector<Metric> userMetrics;
    userMetrics.reserve(userGivenOneShotMetrics.size() + userGivenTwoShotMetrics.size() + userGivenPollingMetrics.size() + userGivenCalculationMetrics.size());
    userMetrics.insert(userMetrics.end(), userGivenOneShotMetrics.begin(), userGivenOneShotMetrics.end());
    userMetrics.insert(userMetrics.end(), userGivenTwoShotMetrics.begin(), userGivenTwoShotMetrics.end());
    userMetrics.insert(userMetrics.end(), userGivenPollingMetrics.begin(), userGivenPollingMetrics.end());
    userMetrics.insert(userMetrics.end(), userGivenCalculationMetrics.begin(), userGivenCalculationMetrics.end());
    return userMetrics;
}

const std::string& Device::getName() const
{
    return this->name;
}

bool Device::operator==(const Device& rhs) const
{
    return name == rhs.name;
}

bool Device::operator!=(const Device& rhs) const
{
    return !(rhs == *this);
}
