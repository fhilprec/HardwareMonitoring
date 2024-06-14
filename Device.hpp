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
public:
    enum MetricType {
        CYCLES,
        INSTRUCTIONS
    };

    struct read_format {
        uint64_t value;
        uint64_t time_enabled;
        uint64_t time_running;
        uint64_t id;
    };

    CPUPerf() {
        allowedMetrics.emplace(Metric(TWO_SHOT, "cycles"), CYCLES);
        //allowedMetrics.emplace(Metric(TWO_SHOT, "instructions"), INSTRUCTIONS);
    }

    void start() {
        fillVector();
    }

    void stop() {
        fillVector(false);
    }

    std::vector<std::pair<Metric, double>> getData(bool isPolling = false) override {
        // TODO: Implement the actual data fetching based on polling if needed
        return {};
    }

    void printVector() {
        for (size_t i = 0; i < prev.size(); i++) {
            std::cout << static_cast<double>(current[i].value - prev[i].value) << std::endl;
        }
    }

private:
    std::vector<read_format> prev;
    std::vector<read_format> current;

    void fillVector(bool first = true) {
        if (first) {
            for (const auto& pair : allowedMetrics) {
                prev.push_back(fetchMetric(pair.second));
            }
        } else {
            for (const auto& pair : allowedMetrics) {
                current.push_back(fetchMetric(pair.second));
            }
        }
    }

    read_format fetchMetric(const int& metricNum) {
        struct perf_event_attr pe;
        memset(&pe, 0, sizeof(struct perf_event_attr));
        pe.type = PERF_TYPE_HARDWARE;
        pe.size = sizeof(struct perf_event_attr);
        pe.config = (metricNum == CYCLES) ? PERF_COUNT_HW_CPU_CYCLES : PERF_COUNT_HW_INSTRUCTIONS;
        pe.disabled = true;
        pe.inherit = 1;
        pe.exclude_user = 0;
        pe.exclude_kernel = 0;
        pe.exclude_hv = 0;
        pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;

        struct read_format rf = {0};
        int fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
        if (fd == -1) {
            std::cerr << "Error opening perf event" << std::endl;
            return rf;
        }

        ioctl(fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

        if (read(fd, &rf, sizeof(rf)) == -1) {
            std::cerr << "Error reading perf event" << std::endl;
            close(fd);
            return rf;
        }

        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        close(fd);

        return rf;
    }
};