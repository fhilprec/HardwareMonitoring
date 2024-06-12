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
#include <vector>
#include <sys/ioctl.h>

#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <unistd.h>


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

    Metric(Sampler samplingMethod, std::string name) : samplingMethod(samplingMethod), name(name) {}
    bool operator==(const Metric &rhs) const {
        return samplingMethod == rhs.samplingMethod &&
               name == rhs.name;
    }

    bool operator!=(const Metric &rhs) const {
        return !(rhs == *this);
    }
};


struct MetricHasher
{
  std::size_t operator()(const Metric& metric) const
  {
    using std::size_t;
    using std::hash;
    using std::string;
    return hash<string>()(metric.name);
  }
};



class Device {
    public:
        Device() {

        }
        std::vector<std::pair<Metric, double>> getData(bool isPolling = false);
        virtual double fetchMetric(const int& metricnum) = 0;
        std::vector<Metric> getAllowedMetrics(){
            std::vector<Metric> result;
            for (auto pair : allowedMetrics) {
                result.push_back(pair.first);
            }
            return result;
        }
    std::unordered_map<Metric, int, MetricHasher> allowedMetrics;
};   


class CPUPerf : public Device {
public:
    CPUPerf() {
        //initialize allowed metrics with cycles only for now
        allowedMetrics.insert(std::make_pair<Metric,int>(Metric(TWO_SHOT, "cycles"),0));
        allowedMetrics.insert(std::make_pair<Metric,int>(Metric(TWO_SHOT, "instructions"),1));

    }

    std::vector<std::pair<Metric, double>> getData(bool isPolling = false) {

        std::vector<std::pair<Metric, double>> result;
        //loop over each allowed metric and call fetchMetric on it
        for (auto& pair: allowedMetrics) {
            result.push_back({pair.first, fetchMetric(pair.second)});
        }
        return result;
        
    }

    void printVector(const std::vector<std::pair<Metric, double>>& vector) {
    for (const auto& pair : vector) {
        std::cout << "Metric: " << pair.first.name << ", Value: " << pair.second << std::endl;
        }
    }



    double fetchMetric(const int& metricnum) override {
        switch (metricnum) {
            case 0: // cycles
            {
                struct perf_event_attr pe;
                memset(&pe, 0, sizeof(struct perf_event_attr));
                pe.type = static_cast<uint32_t>(PERF_TYPE_HARDWARE);
                pe.size = sizeof(struct perf_event_attr);
                pe.config = PERF_COUNT_HW_CPU_CYCLES;
                pe.disabled = true;
                pe.inherit = 1;
                pe.inherit_stat = 0;
                pe.exclude_user = !(0b111 & 0b1);
                pe.exclude_kernel = !(0b111 & 0b10);
                pe.exclude_hv = !(0b111 & 0b100);
                pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
                auto ans = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
                if (errno != 0) {
                    std::cerr << "Error opening perf event: " << errno << std::endl;
                }
                return ans;
            }
            case 1: // instructions
            {
                struct perf_event_attr pe;
                memset(&pe, 0, sizeof(struct perf_event_attr));
                pe.type = static_cast<uint32_t>(PERF_TYPE_HARDWARE);
                pe.size = sizeof(struct perf_event_attr);
                pe.config = PERF_COUNT_HW_INSTRUCTIONS;
                pe.disabled = true;
                pe.inherit = 1;
                pe.inherit_stat = 0;
                pe.exclude_user = !(0b111 & 0b1);
                pe.exclude_kernel = !(0b111 & 0b10);
                pe.exclude_hv = !(0b111 & 0b100);
                pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
                auto ans = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
                if (errno != 0) {
                    std::cerr << "Error opening perf event: " << errno << std::endl;
                }
                return ans;
            }
            default:
                return 0;
        }
    }
    
    };