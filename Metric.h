#ifndef METRIC_H
#define METRIC_H
#include <string>

#include "SamplingMethod.h"

struct Metric {
    SamplingMethod samplingMethod;
    std::string name;
    bool raw;

    Metric() = default;
    Metric(const SamplingMethod samplingMethod, std::string name, const bool raw = false) : samplingMethod(samplingMethod), name(std::move(name)), raw(raw){}

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
