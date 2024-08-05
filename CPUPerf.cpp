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

static const std::vector METRICS{
    Metric(TWO_SHOT, "raw_cycles", true),
    Metric(TWO_SHOT, "raw_kcycles", true),
    Metric(TWO_SHOT, "raw_instructions", true),
    Metric(TWO_SHOT, "raw_L1-misses", true),
    Metric(TWO_SHOT, "raw_LLC-misses", true),
    Metric(TWO_SHOT, "raw_branch-misses", true),
    Metric(CALCULATED, "cycles", false),
    Metric(CALCULATED, "kcycles", false),
    Metric(CALCULATED, "instructions", false),
    Metric(CALCULATED, "L1-misses", false),
    Metric(CALCULATED, "LLC-misses", false),
    Metric(CALCULATED, "branch-misses", false)
};

CPUPerf::CPUPerf(const std::vector<Metric>& metricsToCount): Device<CPUPerf>(metricsToCount)
{
    for (auto& metric: Device<CPUPerf>::getUserMetrics()) {
        if(metric.name == "raw_cycles") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        if(metric.name == "raw_kcycles") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        if(metric.name == "raw_instructions") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
        if(metric.name == "raw_L1-misses") registerCounter(PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_MISS<<16));
        if(metric.name == "raw_LLC-misses") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
        if(metric.name == "raw_branch-misses") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
        if(metric.name == "raw_task-clock") registerCounter(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK);
    }


    for (unsigned i = 0; i < events.size(); i++)
    {
        auto& event = events[i];
        event.fd = static_cast<int>(syscall(__NR_perf_event_open, &event.pe, 0, -1, -1, 0));
        if (event.fd < 0)
        {
            std::cerr << "Error opening counters" << std::endl;
            events.resize(0);
            return;
        }
    }
}


void CPUPerf::registerCounter(uint64_t type, uint64_t eventID)
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

std::vector<std::pair<Metric, Measurement>> CPUPerf::getData(const Sampler sampler)
{

    auto result  = Device<CPUPerf>::getData(sampler);
    if constexpr (TWO_SHOT) first = false;
    return  result;
}

Measurement CPUPerf::fetchMetric(const Metric& metric)
{
    std::vector<std::pair<Metric, Measurement>> result;
    const int index = std::distance(userGivenTwoShotMetrics.begin(),
                                    std::ranges::find(userGivenTwoShotMetrics, metric));
    auto& event = events[index];
    if(first)
    {
        ioctl(event.fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(event.fd, PERF_EVENT_IOC_ENABLE, 0);
    }
    read(event.fd, &event.data, sizeof(uint64_t) * 3);
    if(!first)
    {
        ioctl(event.fd, PERF_EVENT_IOC_DISABLE, 0);
    }
    const std::string valueString = std::format("{}|{}|{}",event.data.value,event.data.time_enabled,event.data.time_running);
    return Measurement(valueString);
}



Measurement CPUPerf::calculateMetric(
    const std::unordered_map<std::string, std::unordered_map<Sampler, std::vector<std::vector<std::pair<Metric, Measurement>>>>>& data, const Metric& rawMetric)
{
    uint64_t valuePrev, timeEnabledPrev, timeRunningPrev;
    parseData(data.at("CPUPerf").at(TWO_SHOT).at(0), rawMetric, valuePrev, timeEnabledPrev, timeRunningPrev);

    uint64_t valueAfter, timeEnabledAfter, timeRunningAfter;
    parseData(data.at("CPUPerf").at(TWO_SHOT).at(1), rawMetric, valueAfter, timeEnabledAfter, timeRunningAfter);

    const double multiplexingCorrection = static_cast<double>(timeEnabledAfter- timeEnabledPrev) / (static_cast<double>(timeRunningAfter - timeRunningPrev)+0.01);
    return Measurement(std::format("{}",static_cast<double>(valueAfter - valuePrev) * multiplexingCorrection));
}

void CPUPerf::parseData(const std::vector<std::pair<Metric, Measurement>>& row, const Metric& rawMetric, uint64_t &value,
    uint64_t &time_enabled, uint64_t &time_running)
{
    Measurement rawMeasurement;
    for (const auto &   [metric, measurement] : row)
    {
        if(metric == rawMetric)
        {
            rawMeasurement = measurement;
            continue;
        }
    }
    std::stringstream ss(rawMeasurement.value);
    std::string currentLine;

    getline(ss,currentLine,'|');
    value = std::stoull(currentLine);

    getline(ss,currentLine,'|');
    time_enabled = std::stoull(currentLine);

    getline(ss,currentLine,'|');
    time_running = std::stoull(currentLine);
}


std::unordered_map<std::string, Metric> CPUPerf::getAllDeviceMetricsByName() {
    std::unordered_map<std::string,Metric> result(METRICS.size());
    for (const auto &metric: METRICS){
        result.emplace(metric.name, metric);
    }
    return result;
}

CPUPerf::~CPUPerf() {
    for (auto& event : events)
    {
        close(event.fd);
    }
}

std::string CPUPerf::getDeviceName() {
    return "CPUPerf";
}
