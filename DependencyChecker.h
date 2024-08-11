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
    allDevicesWantedExist(const std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<std::string, std::vector<Metric>>> &wantedMetricsByAllDevices) {
        std::unordered_set<std::string> deviceNames(wantedMetricsByAllDevices.size());
        for (const auto &[device, wantedMetrics]: wantedMetricsByAllDevices){
            deviceNames.insert(device->getName());
        }

        std::string errorMessages;

        for (const auto &[device, wantedMetrics]: wantedMetricsByAllDevices){
            for (const auto &[wantedDeviceName, wantedMetricsForDevice]: wantedMetrics){
                if(!deviceNames.contains(wantedDeviceName)){
                    errorMessages.append(std::format("Metrics wanted from Device '%s' for Calculation of Metrics by Device '%s' but Device '%s' is not registered for Measurement.\n", wantedDeviceName, device->getName(), wantedDeviceName));
                }
            }
        }

        if(!errorMessages.empty()){
            throw std::logic_error(std::format("Some Devices are Missing for Measurement:\n%s", errorMessages));
        }
    }
    
    static bool deviceHasWantedMetric(std::shared_ptr<IDevice>& device, const Metric& metric) {
        const std::unordered_map<std::string,Metric> &allowedMetrics = device->getAllMetricsByName();

        if(allowedMetrics.contains(metric.name)) return false;

        Metric foundMetric = allowedMetrics.at(metric.name);
        if(foundMetric.raw){
            return true;
        }

        const std::vector<Metric> &chosenMetrics = device->getUserMetrics();
        return std::find(chosenMetrics.begin(), chosenMetrics.end(), foundMetric) != std::end(chosenMetrics);

    }
public:
    static void checkDependenciesBetweenDevicesForCalculatedMetrics(const std::vector<std::shared_ptr<IDevice>>& devices){
        std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<std::string, std::vector<Metric>>> wantedMetricsByAllDevices = getAllWantedMetricsByDevices(devices);
        allDevicesWantedExist(wantedMetricsByAllDevices);

        std::unordered_map<std::string, std::shared_ptr<IDevice>> deviceByDeviceName(wantedMetricsByAllDevices.size());
        for (const auto &[device, wantedMetrics]: wantedMetricsByAllDevices)deviceByDeviceName.emplace(device->getName(),device);

        std::string errorMessages;
        
        for (const auto &[device, wantedMetrics]: wantedMetricsByAllDevices){
            for (const auto &[wantedDeviceName, wantedMetricsForDevice]: wantedMetrics){
                std::shared_ptr<IDevice> wantedDevice = deviceByDeviceName.at(wantedDeviceName);
                for (const auto &wantedMetric: wantedMetricsForDevice){
                    if(!deviceHasWantedMetric(wantedDevice, wantedMetric)){
                        errorMessages.append(std::format("Metric '%s' of Device '%s' wanted from Device '%s', but was not registered for Measurement", wantedMetric.name, wantedDevice->getName(), device->getName()));
                    }
                }
            }
        }
        
        if(!errorMessages.empty()){
            throw std::logic_error(std::format("Some Metrics used by other Devices for Calculation are Missing for Measurement:\n%s", errorMessages));
        }
    }

    static std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<std::string, std::vector<Metric>>>
    getAllWantedMetricsByDevices(const std::vector<std::shared_ptr<IDevice>> &devices) {
        std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<std::string, std::vector<Metric>>> wantedMetricsByAllDevices;
        for (const auto& device: devices){
            std::unordered_map<std::string, std::vector<Metric>> wantedMetricByDevice;
            for (const auto &metric: device->getUserMetrics()){
                if(metric.samplingMethod == CALCULATED) {
                    auto neededMetrics = device->getNeededMetricsOfOtherDevicesForCalculatedMetric(metric);
                    for (const auto & [deviceName, neededMetrics] : neededMetrics)
                    {
                        std::vector<Metric> currentWantedMetrics = wantedMetricsByAllDevices[device][deviceName];
                        for (const auto & neededMetric : neededMetrics)
                        {
                            if(std::ranges::find(currentWantedMetrics,neededMetric) != std::end(currentWantedMetrics))
                            {
                                currentWantedMetrics.push_back(neededMetric);
                            }
                        }
                    }
                }
            }
        }
        return wantedMetricsByAllDevices;
    }
};


#endif //HARDWAREMONITORING_DEPENDENCYCHECKER_H
