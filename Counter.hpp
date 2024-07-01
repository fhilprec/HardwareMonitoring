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
#include "Output.hpp"


struct CounterConfig
{
    const std::vector<std::unique_ptr<Device>> &devices;
    const std::chrono::milliseconds &pollingTimeFrame;
    std::optional<std::filesystem::path> outputDirectory;
};

class Counter
{
private:
    CounterConfig counterConfig;

    std::unordered_map<Device, Output> outputForDevice;
    const Metric TIME_METRIC = {POLLING, "Time of Polling"};
    const Metric TIME_TAKEN_POLLING_METRIC = {POLLING, "Time Taken for Polling"};

    std::jthread pollingThread;
    std::condition_variable startCondition;
    std::mutex startLock;
    bool started = false;

    std::vector<Device> slowPollingDevices;

public:
    explicit Counter(const std::vector<std::unique_ptr<Device>>& devices) : Counter({
        devices,
        std::chrono::milliseconds(500),
        {}
    }) {}
    explicit Counter(CounterConfig counterConfig);

    void start();
    void stop();

private:
    void poll(const std::stop_token& stop_token);
    void fetchData(Sampler sampleMethod);
    void checkPollingTime(const Device& device, const std::chrono::milliseconds& timeForPull);
    void finishPolling()
    {

    }
};
