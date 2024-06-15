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


enum Sampler{
    ONE_SHOT,    // Reads at the end of the process
    TWO_SHOT,    // Reads both at the start and the end of the process and returns the difference
    POLLING      // Reads at an interval
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
    std::size_t operator()(const Metric& metric) const {
        return std::hash<std::string>()(metric.name);
    }
};

class Device {
public:
    Device() {}
    virtual std::vector<std::pair<Metric, double>> getData(bool isPolling = false) = 0;
    std::vector<Metric> getAllowedMetrics() {
        std::vector<Metric> result;
        for (const auto& pair : allowedMetrics) {
            result.push_back(pair.first);
        }
        return result;
    }
protected:
    std::unordered_map<Metric, int, MetricHasher> allowedMetrics;
};








class CPUPerf : public Device {
private:

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
    std::vector<std::string> names;


public:
    CPUPerf() {
        registerCounter("cycles", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        registerCounter("kcycles", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, KERNEL);
        registerCounter("instructions", PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
        registerCounter("L1-misses", PERF_TYPE_HW_CACHE, PERF_COUNT_HW_CACHE_L1D|(PERF_COUNT_HW_CACHE_OP_READ<<8)|(PERF_COUNT_HW_CACHE_RESULT_MISS<<16));
        registerCounter("LLC-misses", PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
        registerCounter("branch-misses", PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
        registerCounter("task-clock", PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK);
        registerCounter("task-clock", PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK);


        for (unsigned i=0; i<events.size(); i++) {
         auto& event = events[i];
         event.fd = static_cast<int>(syscall(__NR_perf_event_open, &event.pe, 0, -1, -1, 0));
         if (event.fd < 0) {
            std::cerr << "Error opening counter " << names[i] << std::endl;
            events.resize(0);
            names.resize(0);
            return;
         }
      }
    }

    void registerCounter(const std::string& name, uint64_t type, uint64_t eventID, EventDomain domain = ALL) {
      names.push_back(name);
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
            std::cerr << "Error reading counter " << names[i] << std::endl;
      }
      startTime = std::chrono::steady_clock::now();
    }

    void stop() {
       stopTime = std::chrono::steady_clock::now();
      for (unsigned i=0; i<events.size(); i++) {
         auto& event = events[i];
         if (read(event.fd, &event.data, sizeof(uint64_t) * 3) != sizeof(uint64_t) * 3)
            std::cerr << "Error reading counter " << names[i] << std::endl;
         ioctl(event.fd, PERF_EVENT_IOC_DISABLE, 0);
      }
    }

    std::vector<std::pair<Metric, double>> getData(bool isPolling = false) override {
        // TODO: Implement the actual data fetching based on polling if needed
        return {};
    }
    
    void printVector() {
        for (size_t i = 0; i < events.size(); i++) {
            LOG(events[i].readCounter());
        }
    }
    
    
};