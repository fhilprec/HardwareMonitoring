#include "MonitoringInterface.h"
#include "Monitor.h"
#include <vector>
#include <memory>
#include <chrono>
#include <filesystem>

#include "CPUPerf.h"
#include "IOFile.h"
#include "GPUFile.h"
#include "NIC.h"

static std::unique_ptr<Monitor> g_monitor = nullptr;

extern "C" {

void start_monitoring() {
    if (!g_monitor) {
        std::vector<std::shared_ptr<IDevice>> devices;
        auto* device = new IOFile();
    devices.emplace_back((IDevice*)device);
    auto* device2 = new CPUPerf();
    devices.emplace_back((IDevice*)device2);
    auto* gpuDevice = new GPUFile();
    devices.emplace_back((IDevice*)gpuDevice);
    auto* nic = new NIC("mlx5_0", 1);
    devices.emplace_back((IDevice*)nic);
        std::filesystem::path outputDirectory("testOutput");
        auto fullPath = std::filesystem::absolute(outputDirectory);

        MonitorConfig config{
            devices,
            std::chrono::milliseconds(500),
            fullPath,
            fullPath
        };

        g_monitor = std::make_unique<Monitor>(config);
    }
    g_monitor->start();
}

void stop_monitoring() {
    if (g_monitor) {
        g_monitor->stop();
    }
}

}