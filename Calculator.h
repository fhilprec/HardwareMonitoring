//
// Created by aleks-tu on 6/30/24.
//

#ifndef CALCULATOR_H
#define CALCULATOR_H
#include <unordered_map>

#include "Device.hpp"


class Calculator {
    std::unordered_map<Device, std::unordered_map<Sampler, std::vector<std::pair<Metric, Measurement>>>> rawMeasurementsByDeviceBySamplingMethod;
    std::vector<Device> devicesWithCalculations;
};



#endif //CALCULATOR_H
