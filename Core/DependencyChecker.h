//
// Created by Aleks on 14.07.2024.
//

#ifndef HARDWAREMONITORING_DEPENDENCYCHECKER_H
#define HARDWAREMONITORING_DEPENDENCYCHECKER_H


#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <unordered_set>
#include "Metric.h"
#include "Device.hpp"
#include "Graph.h"
#include "fmt/format.h"

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
                    errorMessages.append(fmt::format("Metrics wanted from Device '{}' for Calculation of Metrics by Device '{}' but Device '{}' is not registered for Measurement.\n", wantedDeviceName, device->getName(), wantedDeviceName));
                }
            }
        }

        if(!errorMessages.empty()){
            throw std::logic_error(fmt::format("Some Devices are Missing for Measurement:\n{}", errorMessages));
        }
    }
    
    static bool deviceHasWantedMetric(std::shared_ptr<IDevice>& device, const Metric& metric) {
        const std::vector<Metric> &chosenMetrics = device->getUserMetrics();
        return std::find(chosenMetrics.begin(), chosenMetrics.end(), metric) != std::end(chosenMetrics);

    }
public:
    static std::vector<std::pair<std::shared_ptr<IDevice>, Metric>> getSortedCalculatedMetrics(
        const std::vector<std::shared_ptr<IDevice>>& devices)
    {
        std::unordered_map<std::string, std::shared_ptr<IDevice>> deviceByDeviceName(devices.size());
        for (const auto &device: devices)deviceByDeviceName.emplace(device->getName(),device);

        Graph<std::pair<std::shared_ptr<IDevice>,Metric>> metricGraph;
        for (const auto & device_ptr : devices)
        {
            for (const auto & userMetric : device_ptr->getUserMetrics())
            {
                if(userMetric.useForCalculation)
                {
                    std::pair currentMetric = {device_ptr,userMetric};
                    for (const auto & [deviceName, wantedMetrics] : device_ptr->getNeededMetricsOfOtherDevicesForCalculatedMetric(userMetric))
                    {
                        std::vector<std::pair<std::shared_ptr<IDevice>,Metric>> otherMetricNodes;

                        for (const auto & wanted_metric : wantedMetrics)
                        {
                            if(!(wanted_metric == userMetric && wanted_metric.useForMeasurement && wanted_metric.useForCalculation))
                            {
                                otherMetricNodes.emplace_back(deviceByDeviceName.at(deviceName),wanted_metric);
                            }
                        }

                        metricGraph.addEdge(currentMetric, otherMetricNodes);
                    }
                }
            }
        }

        auto result = metricGraph.topologicalSort();
        std::erase_if(result, [](const std::pair<std::shared_ptr<IDevice>,Metric>& metricPair)
        {
            return !metricPair.second.useForCalculation;
        });
        return result;
    }

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
                        errorMessages.append(fmt::format("Metric '{}' of Device '{}' wanted from Device '{}', but was not registered for Measurement", wantedMetric.name, wantedDevice->getName(), device->getName()));
                    }
                }
            }
        }

        if(!errorMessages.empty()){
            throw std::logic_error(fmt::format("Some Metrics used by other Devices for Calculation are Missing for Measurement:\n{}", errorMessages));
        }

        getSortedCalculatedMetrics(devices);
    }

    static std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<std::string, std::vector<Metric>>>
    getAllWantedMetricsByDevices(const std::vector<std::shared_ptr<IDevice>> &devices) {
        std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<std::string, std::vector<Metric>>> wantedMetricsByAllDevices;
        for (const auto& device: devices){
            std::unordered_map<std::string, std::vector<Metric>> wantedMetricByDevice;
            for (const auto &metric: device->getUserMetrics()){
                if(metric.useForCalculation) {
                    auto neededMetrics = device->getNeededMetricsOfOtherDevicesForCalculatedMetric(metric);
                    for (const auto & [deviceName, neededMetrics] : neededMetrics)
                    {
                        std::vector<Metric> currentWantedMetrics = wantedMetricsByAllDevices[device][deviceName];
                        for (const auto & neededMetric : neededMetrics)
                        {
                            if(std::find(currentWantedMetrics.begin(),currentWantedMetrics.end(),neededMetric) != std::end(currentWantedMetrics))
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
