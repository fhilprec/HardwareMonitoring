#pragma once
#include <utility>
#include <vector>
#include <string>

#include "Device.hpp"
#include "OutputConfiguration.hpp"

class Output {
private:
    OutputConfiguration configuration;
    bool isFirstLine = true;

public:
    explicit Output(OutputConfiguration configuration) : configuration(std::move(configuration)) {

    }

    // instead of using std::endl, could flush every x lines
    void writeLine(const std::vector<std::pair<Metric, Measurement>> &measurementResult) {
        if (isFirstLine) {
            for (auto &metric: measurementResult) {
                *configuration.stream_ << metric.first.name << configuration.separator;
            }
            *configuration.stream_ << std::endl;
            isFirstLine = false;
        }
        for (auto &metric: measurementResult) {
            *configuration.stream_ << metric.second.value << configuration.separator;
        }
        *configuration.stream_ << std::endl;
    }
};
