#pragma once

#include <string>
#include <vector>
#include <utility>
#include <chrono>

#include "Device.hpp"

class IOFile final : public Device {
public:
    IOFile() : IOFile(getAllowedMetrics()) {}
    explicit IOFile(const std::vector<Metric>& metrics);
    ~IOFile() override = default;

    std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler) override;
    Measurement fetchMetric(const Metric &metric) override;

private:
    std::string readProcSelfIO();
};
