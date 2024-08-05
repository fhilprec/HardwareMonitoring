#pragma once

#include <functional>

#include "Measurement.h"
#include "Metric.h"

class CalculateMetric : public Metric{
public:
    std::function<Measurement(std::unordered_map<std::string, std::unordered_map<Sampler, std::vector<std::vector<std::pair<Metric, Measurement>>>>> )> calculateMetric;
    std::unordered_map<std::string, std::vector<Metric>> wantedMetricsByDeviceName;

public:
    CalculateMetric(const std::string &name, std::function<Measurement(
            std::unordered_map<std::string, std::unordered_map<Sampler, std::vector<std::vector<std::pair<Metric, Measurement>>>>>)> calculateMetric,
                    std::unordered_map<std::string, std::vector<Metric>> wantedMetrics);
};
