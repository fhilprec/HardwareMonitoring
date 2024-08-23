#ifndef CALCULATOR_H
#define CALCULATOR_H
#include <unordered_map>

#include "Counter.hpp"
#include "DependencyChecker.h"
#include "Device.hpp"
#include "FileManager.h"
#include "fmt/chrono.h"


class Calculator {
    std::vector<std::shared_ptr<IDevice>> devicesWithCalculations;

public:
    explicit Calculator(const std::vector<std::shared_ptr<IDevice>>& devices_with_calculations)
        : devicesWithCalculations(devices_with_calculations)
    {
    }

    std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<bool,std::vector<std::unordered_map<Metric, Measurement>>>>> getRawMeasurementsOfDevices(FileManager& fileManager)
    {
        std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<bool,std::vector<std::unordered_map<Metric, Measurement>>>>> res;
        for (const auto & device : devicesWithCalculations)
        {
            res.emplace(device->getName(),std::unordered_map<SamplingMethod, std::unordered_map<bool,std::vector<std::unordered_map<Metric, Measurement>>>>());

            std::vector<std::unordered_map<Metric, Measurement>>  rawResults = fileManager.readAllFromBuffer(device);
            if(rawResults.empty()) continue;

            for (const auto & line : rawResults)
            {
                SamplingMethod sampler = getSamplerFromDisplayName(line.at(SAMPLING_METHOD_METRIC).value);
                res[device->getName()][sampler][false].push_back(line);
            }
        }
        return res;
    }

    void calculateAndWrite(FileManager fileManager)
    {
        std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::unordered_map<bool, std::vector<std::unordered_map<Metric, Measurement>>>>> allMetrics = getRawMeasurementsOfDevices(fileManager);
        std::vector<std::pair<std::shared_ptr<IDevice>, Metric>> calculatedMetricsOrdered = DependencyChecker::getSortedCalculatedMetrics(devicesWithCalculations);
        for (const auto & [device,metric] : calculatedMetricsOrdered)
        {
            size_t timestamps = allMetrics.at(device->getName()).at(metric.samplingMethod).at(false).size();
auto t = allMetrics.at(device->getName()).at(metric.samplingMethod).at(false);
            for (int i = 0; i < timestamps; ++i)
            {
                std::vector<std::pair<Metric, Measurement>> calculatedMetrics;
                auto result = device->calculateMetric(metric, allMetrics, i);
                auto &calculatedMetricsForDevice = allMetrics.at(device->getName()).at(metric.samplingMethod)[true];
                calculatedMetricsForDevice.emplace_back();
                calculatedMetricsForDevice[i][metric] = result;
                calculatedMetricsForDevice[i][TIME_METRIC] = allMetrics.at(device->getName()).at(metric.samplingMethod).at(false).at(i).at(TIME_METRIC);
                calculatedMetricsForDevice[i][SAMPLING_METHOD_METRIC] = Measurement("CALCULATED");
            }
        }
        std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<SamplingMethod, std::unordered_map<bool, std::vector<std::unordered_map<Metric, Measurement>>>>> res;
        for (const auto & device : devicesWithCalculations)
        {
            res[device].insert(allMetrics.at(device->getName()).begin(),allMetrics.at(device->getName()).end());
        }


        fileManager.save(res);
    }


};



#endif //CALCULATOR_H
