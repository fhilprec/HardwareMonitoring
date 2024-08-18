#pragma once
#include <vector>
#include <linux/perf_event.h>
#include <chrono>

#include "Device.hpp"


enum EventDomain : uint8_t { USER = 0b1, KERNEL = 0b10, HYPERVISOR = 0b100, ALL = 0b111 };

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

class CPUPerf  : public Device<CPUPerf>
{
    bool first = true;
    std::vector<event> events;

public:
    CPUPerf():CPUPerf(std::vector<Metric>()){}
    explicit CPUPerf(const std::vector<Metric>& metricsToCount);
    ~CPUPerf() override;

    std::vector<std::pair<Metric, Measurement>> getData(SamplingMethod sampler) override ;
    Measurement fetchMetric(const Metric &metric) override ;
    Measurement calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::unordered_map<Metric, Measurement>>>> &requestedMetricsByDeviceBySamplingMethod) override;

    static std::string getDeviceName();
    static std::unordered_map<std::string,Metric> getAllDeviceMetricsByName();
    static std::unordered_map<std::string, std::vector<Metric>> getNeededMetricsForCalculatedMetrics(const Metric& metric);

    void registerCounter(uint64_t type, uint64_t eventID);
    static void parseData(const std::unordered_map<Metric, Measurement>& row, const Metric& rawMetric, uint64_t& value, uint64_t&
                          time_enabled, uint64_t& time_running);
};
