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
    std::vector<Device> devices;
    std::chrono::milliseconds pollingTimeFrame;

    std::unordered_map<Device, std::vector<std::vector<std::pair<Metric, double>>>, DeviceHasher> monitoringData;

    std::jthread pollingThread;
    std::condition_variable startCondition;
    std::mutex startLock;
    bool started = false;

public:
    Counter(std::vector<Device> devicesToMonitor, std::chrono::milliseconds pollingTimeFrame) : pollingTimeFrame(
            pollingTimeFrame), devices(std::move(devicesToMonitor)), monitoringData(
            std::unordered_map<Device, std::vector<std::vector<std::pair<Metric, double>>>, DeviceHasher>(
                    devicesToMonitor.size())),pollingThread(
                                                                                                        std::jthread(std::bind_front(
                                                                                                                &Counter::poll,
                                                                                                                this))) {
        for (const auto &device: devices) {
            std::vector<std::vector<std::pair<Metric, double>>> empty_vector(100);
            monitoringData.emplace(device, empty_vector);
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
    }

private:
    void poll(const std::stop_token &stop_token) {
        std::unique_lock<std::mutex> lk(startLock);
        startCondition.wait(lk, [this] { return started; });

        while (!stop_token.stop_requested()) {
            for (Device &device: devices) {
                monitoringData.at(device).push_back(device.getData(true));
            }

            std::this_thread::sleep_for(pollingTimeFrame);
        }

        save();
    }

    void save() {
        for (const auto &device: devices) {
            std::cout << "\nDevice: " << device.name << std::endl;
            auto &deviceData = monitoringData.at(device);
            for (int i = 0; i < deviceData.size(); i++) {
                std::cout << "Time " << pollingTimeFrame.count() * (i + 1) << "ms: " << std::endl;
                printVector(deviceData.at(i));
            }
        }
    }

    //For Testing
    //write a print for a std::vector<std::pair<Metric, double>>
    void printVector(const std::vector<std::pair<Metric, double>> &vector) {
        for (const auto &pair: vector) {
            std::cout << "Metric: " << pair.first.name << ", Value: " << pair.second << std::endl;
        }
    }
};

