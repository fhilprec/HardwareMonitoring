#include "Device.hpp"

#include <utility>


Device::Device(const std::vector<Metric>& rawMetrics, const std::vector<CalculateMetric>& calculatedMetrics,
               const std::vector<Metric>& userMetrics,
               std::string name) : allowedMetrics(rawMetrics), allowedCalculatedMetrics(calculatedMetrics),
                                   name(std::move(name))
{
    for (const auto& allowedMetric : allowedMetrics)
    {
        if (allowedMetric.samplingMethod == CALCULATED) throw std::invalid_argument(
            std::format("Raw Metric '%s' should not have type 'Calculated'", allowedMetric.name));
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
        return;
    }

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
            "This is not the intended way to get calculcated Metrics, calculated metrics are registered with the device but only calculated after fetching the raw metrics");
    }
    return result;
}

std::vector<Metric> Device::getAllowedMetrics()
{
    std::vector<Metric> allAllowedMetrics(allowedMetrics.size() + allowedCalculatedMetrics.size());
    allAllowedMetrics.insert(allAllowedMetrics.end(), allowedMetrics.begin(), allowedMetrics.end());
    allAllowedMetrics.insert(allAllowedMetrics.end(), allowedCalculatedMetrics.begin(), allowedCalculatedMetrics.end());
    return allAllowedMetrics;
}

const std::vector<CalculateMetric>& Device::getCalculatableMetrics() const
{
    return userGivenCalculationMetrics;
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
