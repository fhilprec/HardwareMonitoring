#ifndef CALCULATOR_H
#define CALCULATOR_H
#include <unordered_map>

#include "Counter.hpp"
#include "Device.hpp"
#include "FileManager.h"


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
        std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>> rawMeasurementsByDeviceBySamplingMethod = getRawMeasurementsOfDevices(fileManager);
        for (const auto & device : devicesWithCalculations)
        {
            std::vector<std::pair<Metric, Measurement>> calculatedMetrics;
            for (const auto & metric : device->getUserMetrics())
            {
                if(metric.samplingMethod == CALCULATED) {
                    auto result = device->calculateMetric(metric, rawMeasurementsByDeviceBySamplingMethod);
                    calculatedMetrics.emplace_back(metric,result);
                }
            }
            calculatedMetrics.emplace_back(TIME_METRIC, std::format("{:%Y/%m/%d %T}", std::chrono::system_clock::now()));
            calculatedMetrics.emplace_back(SAMPLING_METHOD_METRIC, getDisplayForSampler(CALCULATED));
            res[device][CALCULATED].push_back(calculatedMetrics);
        }

        for (const auto & device : devicesWithCalculations)
        {
            res[device].insert(rawMeasurementsByDeviceBySamplingMethod[device->getName()].begin(), rawMeasurementsByDeviceBySamplingMethod[device->getName()].end());
        }

        fileManager.save(res);
    }


};



#endif //CALCULATOR_H
