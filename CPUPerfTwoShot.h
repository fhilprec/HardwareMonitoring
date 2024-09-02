#pragma once
#include <vector>
#include <linux/perf_event.h>
#include <chrono>
#include <unordered_map>

#include "Device.hpp"

struct event {
    struct read_format {
        uint64_t value;
        uint64_t time_enabled;
        uint64_t time_running;
        uint64_t id;
    };

    perf_event_attr pe;
    int fd;
    read_format prev;
    read_format data;
};

class CPUPerfTwoShot : public Device<CPUPerfTwoShot>
{
    bool first = true;
    std::vector<event> events;

public:
    CPUPerfTwoShot() : CPUPerfTwoShot(std::vector<Metric>()) {}
    explicit CPUPerfTwoShot(const std::vector<Metric>& metricsToCount);
    ~CPUPerfTwoShot() override;

    std::vector<std::pair<Metric, Measurement>> getData(SamplingMethod sampler) override;
    Measurement fetchMetric(const Metric &metric) override;
    Measurement calculateMetric(const Metric& metric,
                                const std::unordered_map<std::string, std::unordered_map<SamplingMethod,  std::unordered_map<bool,std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                requestedMetricsByDeviceBySamplingMethod,
                                size_t timeIndexForMetric) override;

    static std::string getDeviceName();
    static std::unordered_map<std::string, Metric> getAllDeviceMetricsByName();
    static std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsForCalculatedMetrics(const Metric& metric);

    void registerCounter(uint64_t type, uint64_t eventID);
    static void parseData(const std::unordered_map<Metric, Measurement>& row, const Metric& rawMetric, uint64_t& value, uint64_t&
                          time_enabled, uint64_t& time_running);
};