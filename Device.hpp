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

    private:
        std::vector<Metric> allowedMetrics;
};   
