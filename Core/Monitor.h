//
// Created by aleks-tu on 7/1/24.
//

#ifndef MONITOR_H
#define MONITOR_H
#include "Calculator.h"
#include "Counter.hpp"
#include "FileManager.h"
#include "DependencyChecker.h"
#include <fmt/core.h>


struct MonitorConfig{
    std::vector<std::shared_ptr<IDevice>> devices;
    std::chrono::milliseconds timeFrame;
    std::filesystem::path rawResultsOutputDirectory;
    std::filesystem::path finalResultsOutputDirectory;
};

class Monitor {
private:
    std::vector<std::shared_ptr<IDevice>> devices;

    Counter counter;
    Calculator calculator;
    FileManager fileManager;

    bool running = false;
public:
    explicit Monitor(const MonitorConfig& config)
        : devices(config.devices), counter(Counter({devices,config.timeFrame}, fileManager)), fileManager(FileManager(
            devices, config.rawResultsOutputDirectory, config.finalResultsOutputDirectory)), calculator(Calculator(devices))
    {
        DependencyChecker::checkDependenciesBetweenDevicesForCalculatedMetrics(devices);
    }

    void start();
    void stop();
};



#endif //MONITOR_H
