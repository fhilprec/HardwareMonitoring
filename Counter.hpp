#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <functional>
#include <filesystem>
#include <fstream>

#include "Device.hpp"
#include "FileManager.h"
#include "Output.hpp"


struct CounterConfig
{
    const std::vector<std::shared_ptr<IDevice>> &devices;
    const std::chrono::milliseconds &pollingTimeFrame;
};

static const Metric SAMPLING_METHOD_METRIC = {CALCULATED, "The Sampling Method used"};
static const Metric TIME_METRIC = {POLLING, "Time of Polling"};
static const Metric TIME_TAKEN_POLLING_METRIC = {POLLING, "Time Taken for Polling"};

class Counter
{
private:
    CounterConfig counterConfig;

    FileManager &fileManager;

    std::jthread pollingThread;
    std::condition_variable startCondition;
    std::mutex startLock;
    bool started = false;

    std::vector<std::shared_ptr<IDevice>> slowPollingDevices;

public:
    Counter(const std::vector<std::shared_ptr<IDevice>>& devices, FileManager& fileManager) : Counter({
        devices,
        std::chrono::milliseconds(500)
    }, fileManager) {}
    Counter(CounterConfig counterConfig, FileManager& fileManager);

    void start();
    void stop();

public:
    static std::vector<Metric> getAdditionalMetricsAdded();

private:
    void poll(const std::stop_token& stop_token);
    void fetchData(SamplingMethod sampleMethod);
    void checkPollingTime(const std::shared_ptr<IDevice>& device, const std::chrono::milliseconds& timeForPull);
};
