#include <iostream>
#include <vector>
#include "Device.hpp"
#include <unordered_map>


class Counter {
    public:
        Counter(){

        }
        void start(){}
        void stop(){}
    private:
        std::vector<Device> vec;

        std::unordered_map<Device, std::vector<std::vector<std::pair<Metric, double>>>> monitoringData;
        
        
        /*buffer.append((vec[0].getdata());*/
};

