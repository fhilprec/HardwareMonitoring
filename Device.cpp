#include "Device.hpp"

#include <utility>

template<typename ActualDevice>
Device<ActualDevice>::Device(const std::vector<Metric> &userMetrics) :userMetrics(userMetrics){
    static_assert(std::is_base_of<Device, ActualDevice>::value, "Template class should be the derived class of Device itself");
    static_assert(std::is_nothrow_convertible<typename ActualDevice::getAllDeviceMetricsByName,IDevice::getAllMetricsByNameType>::value_type, "Should implement 'static std::unordered_map<std::string,Metric> getAllMetricsByName()'");
    static_assert(std::is_nothrow_convertible<typename ActualDevice::getDeviceName,IDevice::getNameType>::value_type , "Should implement 'static std::string getName()'");
    initMetrics(userMetrics);
}

template<typename ActualDevice>
void Device<ActualDevice>::initMetrics(const std::vector<Metric> &metricsToCount) {
    std::vector<Metric> metricsToUse;
    if(metricsToCount.empty()){
        for (const auto &[name,metric]: ActualDevice::getAllMetricsByName()){
            metricsToUse.push_back(metric);
        }
    }else{
        for (const auto &metric: metricsToCount){
            if(ActualDevice::getAllowedMetrics().contains(metric.name)){
                metricsToUse.push_back(metric);
            }
        }
        for (const auto &[name,metric]: ActualDevice::getAllMetricsByName()){
            if(metric.isRaw && std::find(metricsToUse.begin(), metricsToUse.end(),metric) != std::end(metricsToUse)){
                metricsToUse.push_back(metric);
            }
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
}

template<typename ActualDevice>
std::vector<std::pair<Metric, Measurement>> Device<ActualDevice>::getData(Sampler sampler) {
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
                    "This is not the intended way to get calculcated Metrics, calculated metrics are registered with the device but only calculated after fetching the (raw) metrics");
    }
    return result;
}

template<typename ActualDevice>
std::vector<Metric> Device<ActualDevice>::getUserMetrics() const {
    return userMetrics;
}

bool IDevice::operator==(const IDevice &rhs) const {
    return this->getName() == rhs.getName();
}
