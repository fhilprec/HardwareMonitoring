#pragma once

#include <iostream>
#include <vector>
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <unistd.h>

#define LOG(x) std::cout << x << std::endl


enum Sampler {
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

    Metric(Sampler samplingMethod, std::string name) : samplingMethod(samplingMethod), name(name) {}

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

struct Measurement {
    Measurement(std::string value) : value(value) {}
    std::string value;
};

class Device {
protected:
    std::vector<Metric> pollingMetrics;
    std::vector<Metric> oneShotMetrics;
    std::vector<Metric> twoShotMetrics;
public:

    Device() = default;
    Device(const std::vector<Metric>& metrics){
        for (const auto &metric: metrics){
            switch (metric.samplingMethod) {
                case ONE_SHOT:
                    oneShotMetrics.push_back(metric);
                    break;
                case TWO_SHOT:
                    twoShotMetrics.push_back(metric);
                    break;
                case POLLING:
                    pollingMetrics.push_back(metric);
                    break;
            }
        }
    } ;

    virtual std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler) {
        std::vector<std::pair<Metric, Measurement>> result;
        switch (sampler) {
            case POLLING:
                for (const auto &pollingMetric: pollingMetrics) {
                    result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
                }
                break;
            case ONE_SHOT:
                for (const auto &pollingMetric: oneShotMetrics) {
                    result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
                }
                break;
            case TWO_SHOT:
                for (const auto &pollingMetric: twoShotMetrics) {
                    result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
                }
                break;
        }
        return result;
    }

    virtual Measurement fetchMetric(const Metric &metric) { return {""}; };


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
    size_t operator()(const Device &device) const {
        return std::hash<std::string>{}(device.name);
    }
};











class CPUPerf : public Device {
private:

    bool first = true;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> stopTime;
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

      double readCounter() {
         double multiplexingCorrection = static_cast<double>(data.time_enabled - prev.time_enabled) / static_cast<double>(data.time_running - prev.time_running);
         return static_cast<double>(data.value - prev.value) * multiplexingCorrection;
      }
   };

    std::vector<event> events;


public:
    CPUPerf() {
        name = "CPUPerf";

        /* Maybe this should not be here */
        std::vector<Metric> pollingMetrics = {Metric(TWO_SHOT, "cycles"),
                                         Metric(TWO_SHOT, "kcycles"),
                                         Metric(TWO_SHOT, "instructions"),
                                         Metric(TWO_SHOT, "L1-misses"),
                                         Metric(TWO_SHOT, "LLC-misses"),
                                         Metric(TWO_SHOT, "branch-misses")};

        twoShotMetrics = pollingMetrics;



        for (auto& metric: twoShotMetrics) {
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

    ~CPUPerf() {
      for (auto& event : events) {
         close(event.fd);
      }
   }

    void registerCounter(uint64_t type, uint64_t eventID, EventDomain domain = ALL) {
      events.push_back(event());
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

    void start() {
        for (unsigned i=0; i<events.size(); i++) {
         auto& event = events[i];
         ioctl(event.fd, PERF_EVENT_IOC_RESET, 0);
         ioctl(event.fd, PERF_EVENT_IOC_ENABLE, 0);
         if (read(event.fd, &event.prev, sizeof(uint64_t) * 3) != sizeof(uint64_t) * 3)
            std::cerr << "Error reading counter " << twoShotMetrics[i].name << std::endl;
      }
      startTime = std::chrono::steady_clock::now();
    }

    void stop() {
       stopTime = std::chrono::steady_clock::now();
      for (unsigned i=0; i<events.size(); i++) {
         auto& event = events[i];
         if (read(event.fd, &event.data, sizeof(uint64_t) * 3) != sizeof(uint64_t) * 3)
            std::cerr << "Error reading counter " << twoShotMetrics[i].name << std::endl;
         ioctl(event.fd, PERF_EVENT_IOC_DISABLE, 0);
      }
    }

    std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler) override  {
        if (sampler == TWO_SHOT && first) {
            start();
            first = false;
            return {};
        }
        return Device::getData(sampler);
    }


    Measurement fetchMetric(const Metric &metric) override {
        std::vector<std::pair<Metric, Measurement>> result;
        int index = std::distance(std::find(twoShotMetrics.begin(), twoShotMetrics.end(),metric),twoShotMetrics.begin());
        auto& event = events[index];
        std::string valueString = std::to_string(first ? 0 : event.readCounter());
        return {valueString};
    }

    void printVector() {
        for (size_t i = 0; i < events.size(); i++) {
            LOG(events[i].readCounter());
        }
    }


};
