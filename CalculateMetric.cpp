#include "CalculateMetric.h"

#include <utility>


CalculateMetric::CalculateMetric(const std::string &name, std::function<Measurement(
        std::unordered_map<std::string, std::unordered_map<Sampler, std::vector<std::vector<std::pair<Metric, Measurement>>>>>)> calculateMetric,
                                 std::unordered_map<std::string, std::vector<Metric>> wantedMetrics) : Metric(CALCULATED, name), calculateMetric(std::move(calculateMetric)),
                                                                                                  wantedMetricsByDeviceName(std::move(wantedMetrics)) {}