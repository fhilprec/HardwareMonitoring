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

    std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>> getRawMeasurementsOfDevices(FileManager& fileManager)
    {
        std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>>  res;
        for (const auto & device : devicesWithCalculations)
        {
            res.emplace(device->getName(),std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>());

            std::vector<std::vector<std::pair<Metric, Measurement>>> rawResults = fileManager.readAllFromBuffer(device);
            if(rawResults.empty()) continue;

            for (const auto & line : rawResults)
            {
                SamplingMethod sampler = line.at(0).first.samplingMethod; // could also be fetched from SampleMethod column
                res[device->getName()][sampler].push_back(line);
            }
        }
        return res;
    }

    void calculateAndWrite(FileManager fileManager)
    {
        std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>> res;
        std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>> allMetrics = getRawMeasurementsOfDevices(fileManager);
        std::vector<std::pair<std::shared_ptr<IDevice>, Metric>> calculatedMetricsOrdered = DependencyChecker::getSortedCalculatedMetrics(devicesWithCalculations);
        for (const auto & [device,metric] : calculatedMetricsOrdered)
        {
            std::vector<std::pair<Metric, Measurement>> calculatedMetrics;
            auto result = device->calculateMetric(metric, allMetrics);
            calculatedMetrics.emplace_back(metric,result);
            calculatedMetrics.emplace_back(TIME_METRIC, fmt::format("{}", std::chrono::system_clock::now()));
            calculatedMetrics.emplace_back(SAMPLING_METHOD_METRIC, getDisplayForSampler(CALCULATED));
            allMetrics.at(device->getName())[CALCULATED].push_back(calculatedMetrics);
            res[device][CALCULATED].push_back(calculatedMetrics);
        }

        for (const auto & device : devicesWithCalculations)
        {
            res[device].insert(allMetrics[device->getName()].begin(), allMetrics[device->getName()].end());
        }

        fileManager.save(res);
    }


};



#endif //CALCULATOR_H
