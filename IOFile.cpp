#include "IOFile.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

IOFile::IOFile(const std::vector<Metric>& metrics) : Device("IOFile", metrics) {}

std::vector<std::pair<Metric, Measurement>> IOFile::getData(Sampler sampler) {
    std::vector<std::pair<Metric, Measurement>> result;
    switch (sampler) {
        case POLLING:
            for (const auto& pollingMetric : pollingMetrics) {
                result.emplace_back(pollingMetric, fetchMetric(pollingMetric));
            }
        break;
        case ONE_SHOT:
            for (const auto& oneShotMetric : oneShotMetrics) {
                result.emplace_back(oneShotMetric, fetchMetric(oneShotMetric));
            }
        break;
        case TWO_SHOT:
            for (const auto& twoShotMetric : twoShotMetrics) {
                result.emplace_back(twoShotMetric, fetchMetric(twoShotMetric));
            }
        break;
    }
    return result;
}

Measurement IOFile::fetchMetric(const Metric &metric) {
    std::string fileContents = readProcSelfIO();
    return Measurement(fileContents);
}

std::string IOFile::readProcSelfIO() {
    std::ifstream file("/proc/self/io");
    if (!file.is_open()) {
        std::cerr << "Failed to open /proc/self/io" << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}
