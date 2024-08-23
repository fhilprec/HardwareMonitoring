#ifndef METRIC_H
#define METRIC_H
#include <string>
#include <utility>

#include "SamplingMethod.h"

struct Metric
{
    SamplingMethod samplingMethod;
    std::string name;
    bool useForMeasurement = true;
    bool useForCalculation = false;
    bool showInResult = true;

    Metric() = default;

    Metric(const SamplingMethod SamplingMethod, std::string Name)
        : samplingMethod(SamplingMethod),
          name(std::move(Name))
    {
    }

    Metric(const SamplingMethod SamplingMethod, const std::string& Name, const bool UseForCalculation)
        : samplingMethod(SamplingMethod),
          name(Name),
          useForCalculation(UseForCalculation)
    {
    }

    Metric(const SamplingMethod SamplingMethod, const std::string& Name, const bool UseForMeasurement,
           const bool UseForCalculation, const bool ShowInResult)
        : samplingMethod(SamplingMethod),
          name(Name),
          useForMeasurement(UseForMeasurement),
          useForCalculation(UseForCalculation),
          showInResult(ShowInResult)
    {
    }

    friend bool operator==(const Metric& Lhs, const Metric& Rhs)
    {
        return Lhs.samplingMethod == Rhs.samplingMethod
            && Lhs.name == Rhs.name
            && Lhs.useForMeasurement == Rhs.useForMeasurement
            && Lhs.useForCalculation == Rhs.useForCalculation
            && Lhs.showInResult == Rhs.showInResult;
    }

    friend bool operator!=(const Metric& Lhs, const Metric& Rhs)
    {
        return !(Lhs == Rhs);
    }

    friend std::size_t hash_value(const Metric& Obj)
    {
        std::size_t seed = 0x77CE1DF8;
        seed ^= (seed << 6) + (seed >> 2) + 0x01446E3D + static_cast<std::size_t>(Obj.samplingMethod);
        seed ^= (seed << 6) + (seed >> 2) + 0x233EE9A9 + std::hash<std::string>{}(Obj.name);
        seed ^= (seed << 6) + (seed >> 2) + 0x39AE2BB3 + static_cast<std::size_t>(Obj.useForMeasurement);
        seed ^= (seed << 6) + (seed >> 2) + 0x761BFB28 + static_cast<std::size_t>(Obj.useForCalculation);
        seed ^= (seed << 6) + (seed >> 2) + 0x18242512 + static_cast<std::size_t>(Obj.showInResult);
        return seed;
    }
};

template <>
struct std::hash<Metric>
{
    size_t operator()(const Metric& metric) const noexcept
    {
        return hash_value(metric);
    }
};
#endif //METRIC_H
