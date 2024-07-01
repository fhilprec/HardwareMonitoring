#include "CalculateMetric.h"


CalculateMetric::CalculateMetric(const std::string& name,
    const std::function<Measurement(std::unordered_map<Device, std::unordered_map<Sampler, std::vector<std::pair<Metric, Measurement>>>>)>&
    calculateMetric) : Metric(CALCULATED, name), calculateMetric(calculateMetric){}