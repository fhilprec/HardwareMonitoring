#ifndef METRIC_H
#define METRIC_H
#include <format>
#include <string>

#include "Sampler.h"

struct Metric {
    Sampler samplingMethod;
    std::string name;

    Metric() = default;
    Metric(const Sampler samplingMethod, std::string name) : samplingMethod(samplingMethod), name(std::move(name)){}

    bool operator==(const Metric &rhs) const {
        return samplingMethod == rhs.samplingMethod &&
               name == rhs.name;
    }

    bool operator!=(const Metric &rhs) const {
        return !(rhs == *this);
    }
};

template <>
struct std::hash<Metric>
{
    size_t operator()(const Metric& metric) const noexcept
    {
        return std::hash<std::string>{}(metric.name);
    }
};
#endif //METRIC_H
