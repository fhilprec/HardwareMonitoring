#include <iostream>
#include <vector>



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
            return syscall(__NR_perf_event_open, 0, 0, -1, -1, 0);
        } 
        if (metric.name == "instructions") {
            return syscall(__NR_perf_event_open, PERF_COUNT_HW_INSTRUCTIONS, 0, -1, -1, 0);
        }
        return 0;
    }
};