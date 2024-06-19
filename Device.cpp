#include "Device.hpp"

#include <utility>

Device::Device(std::string  name, const std::vector<Metric>& metrics): name(std::move(name))
{
    for (const auto& metric : metrics)
    {
        if(std::ranges::find(allowedMetrics, metric) == allowedMetrics.end())
        {
            continue;
        }

        switch (metric.samplingMethod)
        {
        case ONE_SHOT:
            oneShotMetrics.push_back(metric);
            break;
        case TWO_SHOT:
            twoShotMetrics.push_back(metric);
            break;
        case POLLING:
            pollingMetrics.push_back(metric);
            break;
        }
    }
}

std::vector<std::pair<Metric, Measurement>> Device::getData(const Sampler sampler)
{
    std::vector<std::pair<Metric, Measurement>> result;
    switch (sampler)
    {
    case POLLING:
        for (const auto& pollingMetric : pollingMetrics)
        {
            result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
        }
        break;
    case ONE_SHOT:
        for (const auto& pollingMetric : oneShotMetrics)
        {
            result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
        }
        break;
    case TWO_SHOT:
        for (const auto& pollingMetric : twoShotMetrics)
        {
            result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
        }
        break;
    }
    return result;
}

const std::vector<Metric>& Device::getAllowedMetrics() const
{
    return this->allowedMetrics;
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