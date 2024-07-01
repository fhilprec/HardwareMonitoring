#include "CPUPerf.h"

#include <vector>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

CPUPerf::CPUPerf(const std::vector<Metric>& metricsToCount): Device(
    {Metric(TWO_SHOT, "cycles"),
        Metric(TWO_SHOT, "kcycles"),
        Metric(TWO_SHOT, "instructions"),
        Metric(TWO_SHOT, "L1-misses"),
        Metric(TWO_SHOT, "LLC-misses"),
        Metric(TWO_SHOT, "branch-misses")},{},metricsToCount,"CPUPerf")
{
    for (auto& metric: userGivenTwoShotMetrics) {
        if(metric.name == "cycles") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        if(metric.name == "kcycles") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, KERNEL);
        if(metric.name == "instructions") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
        if(metric.name == "L1-misses") registerCounter(PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_MISS<<16));
        if(metric.name == "LLC-misses") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
        if(metric.name == "branch-misses") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
        if(metric.name == "task-clock") registerCounter(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK);
    }


    for (unsigned i=0; i<events.size(); i++) {
        auto& event = events[i];
        event.fd = static_cast<int>(syscall(__NR_perf_event_open, &event.pe, 0, -1, -1, 0));
        if (event.fd < 0) {
            std::cerr << "Error opening counters" << std::endl;
            events.resize(0);
            return;
        }
    }
}



CPUPerf::~CPUPerf()
{
    for (auto& event : events) {
        close(event.fd);
    }
}

void CPUPerf::registerCounter(uint64_t type, uint64_t eventID, EventDomain domain)
{
    events.emplace_back();
    auto& event = events.back();
    auto& pe = event.pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = static_cast<uint32_t>(type);
    pe.size = sizeof(struct perf_event_attr);
    pe.config = eventID;
    pe.disabled = true;
    pe.inherit = 1;
    pe.inherit_stat = 0;
    pe.exclude_user = 0;
    pe.exclude_kernel = 0;
    pe.exclude_hv = 0;
    pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
}

void CPUPerf::start()
{
    for (unsigned i=0; i<events.size(); i++) {
        auto& event = events[i];
        ioctl(event.fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(event.fd, PERF_EVENT_IOC_ENABLE, 0);
        if (read(event.fd, &event.prev, sizeof(uint64_t) * 3) != sizeof(uint64_t) * 3)
            std::cerr << "Error reading counter " << userGivenTwoShotMetrics[i].name << std::endl;
    }
    startTime = std::chrono::steady_clock::now();
}

void CPUPerf::stop()
{
    stopTime = std::chrono::steady_clock::now();
    for (unsigned i=0; i<events.size(); i++) {
        auto& event = events[i];
        if (read(event.fd, &event.data, sizeof(uint64_t) * 3) != sizeof(uint64_t) * 3)
            std::cerr << "Error reading counter " << userGivenTwoShotMetrics[i].name << std::endl;
        ioctl(event.fd, PERF_EVENT_IOC_DISABLE, 0);
    }
}

std::vector<std::pair<Metric, Measurement>> CPUPerf::getData(const Sampler sampler)
{
    if (sampler != TWO_SHOT) {
        return {};
    }

    if (first)
    {
        first = false;
        start();
        return {};
    }

    stop();
    return Device::getData(sampler);
}

Measurement CPUPerf::fetchMetric(const Metric& metric)
{
    std::vector<std::pair<Metric, Measurement>> result;
    const int index = std::distance(userGivenTwoShotMetrics.begin(),std::ranges::find(userGivenTwoShotMetrics,metric));
    const auto& event = events[index];
    const std::string valueString = std::to_string(first ? 0 : event.readCounter());
    return Measurement(valueString);
}
