//
// Created by Aleks on 14.07.2024.
//

#ifndef HARDWAREMONITORING_DEPENDENCYCHECKER_H
#define HARDWAREMONITORING_DEPENDENCYCHECKER_H


#include <vector>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include "Metric.h"
#include "Device.hpp"

class DependencyChecker {
    static void
    allDevicesWantedExist(const std::unordered_map<Device, std::unordered_map<std::string, std::vector<Metric>>> &wantedMetricsByAllDevices) {
        std::unordered_set<std::string> deviceNames(wantedMetricsByAllDevices.size());
        for (const auto &[device, wantedMetrics]: wantedMetricsByAllDevices){
            deviceNames.insert(device.getName());
        }

        std::string errorMessage;

        for (const auto &[device, wantedMetrics]: wantedMetricsByAllDevices){
            for (const auto &[wantedDeviceName, wantedMetricsForDevice]: wantedMetrics){
                if(!deviceNames.contains(wantedDeviceName)){
                    errorMessage.append(std::format("Metrics wanted from Device '%s' for Calculation of Metrics by Device '%s' but Device '%s' is not registered for Measurement.\n", wantedDeviceName, device.getName(), wantedDeviceName));
                }
            }
        }

        if(!errorMessage.empty()){
            throw std::logic_error(std::format("Some Devices are Missing for Measurement:\n%s", errorMessage));
        }
    }

    static void checkDependenciesBetweenDevicesForCalculatedMetrics(const std::unordered_map<Device, std::unordered_map<std::string, std::vector<Metric>>>& wantedMetricsByAllDevices){
        allDevicesWantedExist(wantedMetricsByAllDevices);


    }
};


#endif //HARDWAREMONITORING_DEPENDENCYCHECKER_H
