#ifndef METRIC_H
#define METRIC_H
#include <string>

#include "Sampler.h"

struct Metric {
    Sampler samplingMethod;
    std::string name;

    Metric(const Sampler samplingMethod, std::string name) : samplingMethod(samplingMethod), name(std::move(name)) {}

    bool operator==(const Metric &rhs) const {
        return samplingMethod == rhs.samplingMethod &&
               name == rhs.name;
    }

    bool operator!=(const Metric &rhs) const {
        return !(rhs == *this);
    }
};

struct MetricHasher {
    std::size_t operator()(const Metric &metric) const {
        return std::hash<std::string>()(metric.name);
    }
};
#endif //METRIC_H
