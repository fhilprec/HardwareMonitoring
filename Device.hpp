#pragma once

#include <iostream>
#include <vector>
#include <sys/ioctl.h>

#include <unistd.h>
#include <linux/perf_event.h>
#include <syscall.h>


enum Sampler{
    /* Reads at the end of the process */
    ONE_SHOT,
    /* Reads both at the start and the end of the process and returns the difference */
    TWO_SHOT,
    /* Reads at an interval */
    POLLING
};

struct Metric {
    Sampler samplingMethod;
    std::string name;
};


class Device {
public:
    virtual ~Device();
    Device() = default;

    std::vector<std::pair<Metric, double>> getData(Sampler sampler) {
        std::vector<std::pair<Metric, double>> result;
        //loop over each allowed metric and call fetchMetric on it
        // TODO differentiate between sampling methods, maybe map or different vectors?
        for (Metric metric: allowedMetrics) {
            result.push_back({metric, fetchMetric(metric)});
        }
        return result;

    }
    virtual double fetchMetric(Metric metric){return 0;};


    std::vector<Metric> allowedMetrics;
    std::string name;
    std::vector<Metric> getAllowedMetrics() {
        return this->allowedMetrics;
    }


    bool operator==(const Device &rhs) const {
        return name == rhs.name;
    }

    bool operator!=(const Device &rhs) const {
        return !(rhs == *this);
    }

};

struct DeviceHasher {
    size_t operator()(const Device& device) const{
        return std::hash<std::string>{}(device.name);
    }
};


class CPUPerf : public Device {
public:
    CPUPerf() {
        //initialize allowed metrics with cycles only for now
        allowedMetrics.push_back({TWO_SHOT, "cycles"});
        allowedMetrics.push_back({TWO_SHOT, "instructions"});
        name = "CPUPerf";
    }


    double fetchMetric(Metric metric) override {
        if (metric.name == "cycles") {
            return syscall(SYS_perf_event_open, 0, 0, -1, -1, 0);
        }
        if (metric.name == "instructions") {
            return syscall(SYS_perf_event_open, PERF_COUNT_HW_INSTRUCTIONS, 0, -1, -1, 0);
        }
        return 0;
    }
};