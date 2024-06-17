#include "Device.hpp"
#include "Output.hpp"

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

struct CounterConfig{
    std::vector<std::unique_ptr<Device>> devices;
    std::chrono::milliseconds pollingTimeFrame;
    std::optional<std::filesystem::path> outputDirectory;
};

class Counter {
private:
    CounterConfig counterConfig;

    std::unordered_map<Device, Output ,DeviceHasher> outputForDevice;
    const Metric TIME_METRIC = {POLLING,"Time of Polling"};
    const Metric TIME_TAKEN_POLLING_METRIC = {POLLING,"Time Taken for Polling"};

    std::jthread pollingThread;
    std::condition_variable startCondition;
    std::mutex startLock;
    bool started = false;

    std::vector<Device> slowPollingDevices;

public:
    explicit Counter(std::vector<std::unique_ptr<Device>>& devices) : Counter(devices, std::chrono::milliseconds(500),{}) {}
    Counter(std::vector<std::unique_ptr<Device>>& devicesToMonitor, std::chrono::milliseconds pollingTimeFrame, std::optional<std::filesystem::path> outputDirectory):
    counterConfig({std::move(devicesToMonitor), pollingTimeFrame, std::move(outputDirectory)}),
    outputForDevice(std::unordered_map<Device, Output, DeviceHasher>(devicesToMonitor.size())),
    pollingThread(std::jthread(std::bind_front(&Counter::poll, this))),
    slowPollingDevices(std::vector<Device>(devicesToMonitor.size()))
    {

        if(counterConfig.outputDirectory.has_value() && !std::filesystem::is_directory(counterConfig.outputDirectory.value())){
            throw std::invalid_argument(std::format("Path '{}' is not a directory", counterConfig.outputDirectory.value().string()));
        }
        for (const auto &device: counterConfig.devices) {
            OutputConfiguration configForDevice = {";", false, std::shared_ptr<std::ostream>(&std::cout,[](void*) {})};
            if(counterConfig.outputDirectory.has_value()){
                configForDevice.fileMode = true;
                std::filesystem::path filePath = counterConfig.outputDirectory.value().append(std::format("{}.csv",device->name));
                configForDevice.stream_ = std::make_shared<std::ofstream>(std::ofstream(std::ofstream(filePath)));
            }
            outputForDevice.emplace(*device, Output(configForDevice));
        }
    }

    void start() {
        started = true;
        startCondition.notify_all();
    }

    void stop() {
        pollingThread.request_stop();
        pollingThread.join();
        started = false;
        startCondition.notify_all();
    }

private:
    void poll(const std::stop_token &stop_token) {
        std::unique_lock<std::mutex> lk(startLock);
        startCondition.wait(lk, [this] { return started; });

        fetchData(TWO_SHOT);
        
        while (!stop_token.stop_requested()) {
            auto startTimePoll = std::chrono::system_clock::now();
            fetchData(POLLING);
            auto endTimePoll = std::chrono::system_clock::now();

            auto remainingPollTime = counterConfig.pollingTimeFrame - std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoll - startTimePoll);
            remainingPollTime = remainingPollTime < std::chrono::milliseconds(0) ?  counterConfig.pollingTimeFrame - remainingPollTime : remainingPollTime;
            
            std::this_thread::sleep_for(remainingPollTime);
        }

        fetchData(TWO_SHOT);
        fetchData(ONE_SHOT);
        finish();
    }

    void fetchData(Sampler sampleMethod) {
        for (const auto &device:  counterConfig.devices) {
            auto startPoll = std::chrono::system_clock::now();
            auto deviceData = device->getData(sampleMethod);
            auto endPoll = std::chrono::system_clock::now();

            auto timeForPull = std::chrono::duration_cast<std::chrono::milliseconds>(endPoll - startPoll);

            #if sampleMethod == POLLING
            checkPollingTime(*device, timeForPull);
            #endif

            if(!deviceData.empty()) {
                deviceData.emplace_back(TIME_TAKEN_POLLING_METRIC, std::format("{} ms", timeForPull.count()));
                deviceData.emplace_back(TIME_METRIC, std::format("{:%Y/%m/%d %T}", startPoll));
                outputForDevice.at(*device).writeLine(deviceData);
            }
        }
    }

    void checkPollingTime(const Device &device, const std::chrono::milliseconds &timeForPull) {
        bool deviceNotWarned = std::find(slowPollingDevices.begin(), slowPollingDevices.end(), device) != slowPollingDevices.end();

        if(deviceNotWarned && timeForPull *  counterConfig.devices.size() >  counterConfig.pollingTimeFrame){
            std::cerr << "Polling Time Window of " <<  counterConfig.pollingTimeFrame.count() << "ms may get skipped";
            std::cerr << "Device '" << device.name << "' took " << timeForPull.count() << "ms for Poll, expecting Poll to get skipped";
            slowPollingDevices.push_back(device);
        }
    }

    void finish(){
    }
};

