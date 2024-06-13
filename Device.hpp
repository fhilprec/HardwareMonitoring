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
    enum MetricType {
        CYCLES,
        INSTRUCTIONS
    };

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
         LOG(data.value - prev.value);
         return static_cast<double>(data.value - prev.value) * multiplexingCorrection;
      }
   };



    CPUPerf() {
        //initialize allowed metrics with cycles only for now
        allowedMetrics.insert(std::make_pair<Metric,int>(Metric(TWO_SHOT, "cycles"),CYCLES));
        allowedMetrics.insert(std::make_pair<Metric,int>(Metric(TWO_SHOT, "instructions"),INSTRUCTIONS));

        // memset(&pe, 0, sizeof(struct perf_event_attr));
        // pe.type = static_cast<uint32_t>(PERF_TYPE_HARDWARE);
        // pe.size = sizeof(struct perf_event_attr);
        // //pe.config = PERF_COUNT_HW_CPU_CYCLES; done in fetchMetric
        // pe.disabled = true;
        // pe.inherit = 1;
        // pe.inherit_stat = 0;
        // pe.exclude_user = !(0b111 & 0b1);
        // pe.exclude_kernel = !(0b111 & 0b10);
        // pe.exclude_hv = !(0b111 & 0b100);
        // pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;

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



    double fetchMetric(const int& MetricNum) override {
        long ans;
        switch (MetricNum) {
            case CYCLES: // cycles
            {
                perf_event_attr pe;
                memset(&pe, 0, sizeof(struct perf_event_attr));
                pe.type = static_cast<uint32_t>(PERF_TYPE_HARDWARE);
                pe.size = sizeof(struct perf_event_attr);
                //pe.config = PERF_COUNT_HW_CPU_CYCLES; done in fetchMetric
                pe.disabled = true;
                pe.inherit = 1;
                pe.inherit_stat = 0;
                pe.exclude_user = !(0b111 & 0b1);
                pe.exclude_kernel = !(0b111 & 0b10);
                pe.exclude_hv = !(0b111 & 0b100);
                pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
                pe.config = PERF_COUNT_HW_CPU_CYCLES;
                int ret1 = static_cast<int>(__NR_perf_event_open, &pe, 0, -1, -1, 0);
                uint64_t ret2 = 0;
                read(ret1, &ret2, sizeof(uint64_t) * 1);
                std::cout << ret2 << std::endl;
                return static_cast<double>(ret2);

            }break;
            case INSTRUCTIONS: // instructions
            {
                perf_event_attr pe;
                memset(&pe, 0, sizeof(struct perf_event_attr));
                pe.type = static_cast<uint32_t>(PERF_TYPE_HARDWARE);
                pe.size = sizeof(struct perf_event_attr);
                //pe.config = PERF_COUNT_HW_CPU_CYCLES; done in fetchMetric
                pe.disabled = true;
                pe.inherit = 1;
                pe.inherit_stat = 0;
                pe.exclude_user = !(0b111 & 0b1);
                pe.exclude_kernel = !(0b111 & 0b10);
                pe.exclude_hv = !(0b111 & 0b100);
                pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
                pe.config = PERF_COUNT_HW_CPU_CYCLES;
                pe.config = PERF_COUNT_HW_INSTRUCTIONS;

                int ret1 = (int)syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
                uint64_t ret2 = 0;
                read(ret1, &ret2, sizeof(uint64_t) * 1);
                return static_cast<double>(ret2);
            }break;
            default:
                return 0;
        }
        // ans = syscall(__NR_perf_event_open, pe, 0, -1, -1, 0);
        // long long_ans = 0;
        // read(ans, &long_ans, sizeof(uint64_t) * 1) ;
        // std::cout << long_ans << std::endl;
        // double ret = (double)long_ans;
        // return ret;
    }
    private:
        // struct perf_event_attr pe;//sd
    };

