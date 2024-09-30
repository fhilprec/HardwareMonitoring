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

#include "fmt/format.h"

static const std::vector METRICS{
    Metric(POLLING, "cycles", true),
    Metric(POLLING, "kcycles", true),
    Metric(POLLING, "instructions", true),
    Metric(POLLING, "L1-misses", true),
    Metric(POLLING, "LLC-misses", true),
    Metric(POLLING, "branch-missese", true),
    Metric(POLLING, "task-clock", true)
};

CPUPerf::CPUPerf(const std::vector<Metric>& metricsToCount): Device(metricsToCount)
{
    for (auto& metric: Device::getUserMetrics()) {
        if(metric.name == "cycles") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        if(metric.name == "kcycles") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        if(metric.name == "instructions") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
        if(metric.name == "L1-misses") registerCounter(PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_MISS<<16));
        if(metric.name == "LLC-misses") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
        if(metric.name == "branch-misses") registerCounter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
        if(metric.name == "task-clock") registerCounter(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK);
    }


    for (unsigned i = 0; i < events.size(); i++)
    {
        auto& event = events[i];
        event.fd = static_cast<int>(syscall(__NR_perf_event_open, &event.pe, 0, -1, -1, 0));
        if (event.fd < 0)
        {
            events.resize(0);
            throw std::runtime_error(fmt::format("Error opening counters for {} Device, you may need to set kernel.perf_event_paranoid=-1",getDeviceName()));
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

std::vector<std::pair<Metric, Measurement>> CPUPerf::getData(const SamplingMethod sampler)
{

    auto result  = Device::getData(sampler);
    if (sampler == POLLING) first = false;
    return  result;
}

Measurement CPUPerf::fetchMetric(const Metric& metric)
{
    std::vector<std::pair<Metric, Measurement>> result;
    const size_t index = std::distance(userGivenMeasurementTwoShotMetrics.begin(),
                                    std::find(userGivenMeasurementTwoShotMetrics.begin(), userGivenMeasurementTwoShotMetrics.end(), metric));
    auto& event = events[index];
    if(first)
    {
        ioctl(event.fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(event.fd, PERF_EVENT_IOC_ENABLE, 0);
    }
    read(event.fd, &event.data, sizeof(uint64_t) * 3);
    const std::string valueString = fmt::format("{}|{}|{}",event.data.value,event.data.time_enabled,event.data.time_running);
    return Measurement(valueString);
}

Measurement CPUPerf::calculateMetric(const Metric& metric,
                                     const std::unordered_map<std::string, std::unordered_map<SamplingMethod,  std::unordered_map<bool,std::vector<std::unordered_map<Metric, Measurement>>>>>&
                                     requestedMetricsByDeviceBySamplingMethod,
                                     size_t timeIndexForMetric)
{
    if(timeIndexForMetric == 0) return Measurement("0");

    uint64_t valuePrev, timeEnabledPrev, timeRunningPrev;
    parseData(requestedMetricsByDeviceBySamplingMethod.at(getDeviceName()).at(POLLING).at(false).at(timeIndexForMetric-1), metric, valuePrev, timeEnabledPrev, timeRunningPrev);

    uint64_t valueAfter, timeEnabledAfter, timeRunningAfter;
    parseData(requestedMetricsByDeviceBySamplingMethod.at(getDeviceName()).at(POLLING).at(false).at(timeIndexForMetric), metric, valueAfter, timeEnabledAfter, timeRunningAfter);

    const double multiplexingCorrection = static_cast<double>(timeEnabledAfter- timeEnabledPrev) / (static_cast<double>(timeRunningAfter - timeRunningPrev)+0.01);
    return Measurement(fmt::format("{}",static_cast<double>(valueAfter - valuePrev) * multiplexingCorrection));
}


void CPUPerf::parseData(const std::unordered_map<Metric, Measurement>& row, const Metric& rawMetric, uint64_t &value,
    uint64_t &time_enabled, uint64_t &time_running)
{
    const Measurement& rawMeasurement = row.at(rawMetric);
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

std::unordered_map<std::string, std::vector<Metric>> CPUPerf::getNeededMetricsForCalculatedMetrics(const Metric& metric)
{
    // Could also be done with switch case on metric name
    const size_t metricIndex = std::distance(METRICS.begin(),std::find(METRICS.begin(), METRICS.end(), metric));
    return {{getDeviceName(),{METRICS[metricIndex]}}};
}

CPUPerf::~CPUPerf() {

    for (auto& event : events)
    {
        ioctl(event.fd, PERF_EVENT_IOC_DISABLE, 0);
        close(event.fd);
    }
}

std::string CPUPerf::getDeviceName() {
    return "CPUPerf";
}