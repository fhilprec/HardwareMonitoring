//
// Created by aleks-tu on 7/1/24.
//

#ifndef MONITOR_H
#define MONITOR_H
#include "Calculator.h"
#include "Counter.hpp"
#include "FileManager.h"


class Monitor {
private:
    std::vector<std::shared_ptr<Device>> devices;

    Counter counter;
    Calculator calculator;
    FileManager fileManager;

    bool running = false;
public:
    explicit Monitor(const std::vector<std::shared_ptr<Device>>& devices, const std::optional<std::filesystem::path>& outputDirectory)
        : devices(devices), counter(Counter(devices, fileManager)), fileManager(FileManager(devices, outputDirectory)), calculator(Calculator(devices))
    {
    }

    void start();
    void stop();
};



#endif //MONITOR_H
