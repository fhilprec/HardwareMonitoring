#pragma once

#include <functional>

#include "Measurement.h"
#include "Metric.h"

class Device;

class CalculateMetric : public Metric{
    std::function<Measurement(std::unordered_map<Device, std::unordered_map<Sampler, std::vector<std::pair<Metric, Measurement>>>>)> calculateMetric;

public:
    CalculateMetric(const std::string& name, const std::function<Measurement(std::unordered_map<Device, std::unordered_map<Sampler, std::vector<std::pair<Metric, Measurement>>>>)>& calculateMetric);
};
