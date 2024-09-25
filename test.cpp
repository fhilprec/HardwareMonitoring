#include <vector>

#include "CPUPerf.h"
#include "Monitor.h"

void doWork(){}

int main() {
    std::vector<std::shared_ptr<IDevice>> devices;

    devices.emplace_back(new CPUPerf());

    std::filesystem::path outputDirectory("output");

    MonitorConfig config = {devices, std::chrono::milliseconds(500), outputDirectory, outputDirectory};
    Monitor monitor(config);

    monitor.start();
    doWork();
    monitor.stop();
}