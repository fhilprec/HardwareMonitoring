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

    double readCounter() const
    {
        const double multiplexingCorrection = static_cast<double>(data.time_enabled - prev.time_enabled) / static_cast<double>(data.time_running - prev.time_running);
        return static_cast<double>(data.value - prev.value) * multiplexingCorrection;
    }
};

class CPUPerf final : public Device
{
    bool first = true;
    std::vector<event> events;

public:
    CPUPerf():CPUPerf(std::vector<Metric>()){}
    explicit CPUPerf(const std::vector<Metric>& metricsToCount);
    ~CPUPerf() override;

    std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler) override;
    Measurement fetchMetric(const Metric &metric) override;

    void registerCounter(uint64_t type, uint64_t eventID);
    void start();
    void stop();
    static Measurement calculateMetric(
        const std::unordered_map<std::string, std::unordered_map<
                                     Sampler, std::vector<std::vector<std::pair<Metric, Measurement>>>>>& data,
        const Metric& rawMetric);
    static void parseData(const std::vector<std::pair<Metric, Measurement>>& row, const Metric& rawMetric, uint64_t& value, uint64_t&
                          time_enabled, uint64_t& time_running);
};
