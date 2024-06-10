#include <iostream>
#include <vector>
#include <linux/perf_event.h>
#include <sys/syscall.h>



struct Metric{
    bool isPolling = false;
    std::string name;
};


class Device {
    public:
        Device(){

        }
        std::vector<std::pair<Metric, double>> getData(bool isPolling = false);
        virtual double fetchMetric(Metric metric) = 0;
        std::vector<Metric> getAllowedMetrics(){
            return this->allowedMetrics;
        }
        std::vector<Metric> allowedMetrics;

};   


class CPUPerf : public Device {
public:
    CPUPerf() {
        //initialize allowed metrics with cycles only for now
        allowedMetrics.push_back({false, "cycles"});
        allowedMetrics.push_back({false, "instructions"});
    }

    std::vector<std::pair<Metric, double>> getData(bool isPolling = false) {

        std::vector<std::pair<Metric, double>> result;
        //loop over each allowed metric and call fetchMetric on it
        for (Metric metric : allowedMetrics) {
            result.push_back({metric, fetchMetric(metric)});
        }
        return result;
        
    }

    void printVector(const std::vector<std::pair<Metric, double>>& vector) {
    for (const auto& pair : vector) {
        std::cout << "Metric: " << pair.first.name << ", Value: " << pair.second << std::endl;
        }
    }




    double fetchMetric(Metric metric) override {
        if (metric.name == "cycles") {
        return syscall(SYS_perf_event_open, nullptr, -1, -1, -1, 0);
    }
        if (metric.name == "instructions") {
            struct perf_event_attr pe;
            pe.type = PERF_TYPE_HARDWARE;
            pe.config = PERF_COUNT_HW_INSTRUCTIONS;
            pe.sample_period = 1000;
            pe.sample_type = PERF_SAMPLE_RAW;
            pe.sample_regs_user = PERF_SAMPLE_REGS_USER | PERF_SAMPLE_REGS_INTR;
            pe.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING | PERF_FORMAT_ID;
        return syscall(SYS_perf_event_open, &pe, -1, -1, -1, 0);
    }
    return 0;
    }
};