#include "Device.hpp"

#include <iostream>
#include <utility>
#include <vector>
#include <unordered_map>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <functional>


class Counter {
private:
    std::vector<std::unique_ptr<Device>> devices;
    std::chrono::milliseconds pollingTimeFrame;

    std::unordered_map<Device, std::vector<std::vector<std::pair<Metric, Measurement>>>, DeviceHasher> monitoringData; //TODO replace with output
    const Metric TIME_METRIC = {POLLING,"Time of Polling"};
    const Metric TIME_TAKEN_POLLING_METRIC = {POLLING,"Time Taken for Polling"};

    std::jthread pollingThread;
    std::condition_variable startCondition;
    std::mutex startLock;
    bool started = false;

    std::vector<Device> slowPollingDevices;

public:
    explicit Counter(std::vector<std::unique_ptr<Device>>& devices) : Counter(devices, std::chrono::milliseconds(500)) {}
    Counter(std::vector<std::unique_ptr<Device>>& devicesToMonitor, std::chrono::milliseconds pollingTimeFrame) : pollingTimeFrame(
            pollingTimeFrame), devices(std::move(devicesToMonitor)), monitoringData(
            std::unordered_map<Device, std::vector<std::vector<std::pair<Metric, Measurement>>>, DeviceHasher>(devicesToMonitor.size())),
            pollingThread(std::jthread(std::bind_front(&Counter::poll, this))),
            slowPollingDevices(std::vector<Device>(devicesToMonitor.size())){
        for (const auto &device: devices) {
            std::vector<std::vector<std::pair<Metric, Measurement>>> empty_vector;
            monitoringData.emplace(*device, empty_vector);
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

            auto remainingPollTime = pollingTimeFrame -std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoll - startTimePoll);
            remainingPollTime = remainingPollTime < std::chrono::milliseconds(0) ? pollingTimeFrame - remainingPollTime : remainingPollTime;
            
            std::this_thread::sleep_for(remainingPollTime);
        }

        fetchData(TWO_SHOT);
        fetchData(ONE_SHOT);
        finish();
    }

    void fetchData(Sampler sampleMethod) {
        for (const auto &device: devices) {
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
                monitoringData.at(*device).push_back(deviceData);
            }
        }
    }

    void checkPollingTime(const Device &device, const std::chrono::milliseconds &timeForPull) {
        bool deviceNotWarned = std::find(slowPollingDevices.begin(), slowPollingDevices.end(), device) != slowPollingDevices.end();

        if(deviceNotWarned && timeForPull * devices.size() > pollingTimeFrame){
            std::cerr << "Polling Time Window of " << pollingTimeFrame.count() << "ms may get skipped";
            std::cerr << "Device '" << device.name << "' took " << timeForPull.count() << "ms for Poll, expecting Poll to get skipped";
            slowPollingDevices.push_back(device);
        }
    }

    void finish(){
        //TODO close output?
        //TEST
        for (const auto &device: devices) {
            std::cout << "\nDevice: " << device->name << std::endl;
            auto &deviceData = monitoringData.at(*device);
            for (const auto & i : deviceData) {
                printVector(i);
            }
        }
    }

    //For Testing
    //write a print for a std::vector<std::pair<Metric, double>>
    void printVector(const std::vector<std::pair<Metric, Measurement>> &vector) {
        for (const auto &pair: vector) {
            std::cout << "Metric: " << pair.first.name << ", Value: " << pair.second.value << std::endl;
        }
    }
};

