#include <vector>
#include <string>
#include "Device.hpp"
#include "OutputConfiguration.hpp"

class Output {
private:
    OutputConfiguration configuration;
    bool isFirstLine = true;

public:
    explicit Output(OutputConfiguration &configuration) : configuration(configuration) {

    }

    // instead of using std::endl, could flush every x lines
    void writeLine(const std::vector<std::pair<Metric, Measurement>> &metrics) {
        if (isFirstLine) {
            for (auto &metric: metrics) {
                *configuration.stream_ << metric.first.name;
                if (&metric != &metrics.back()){
                    *configuration.stream_ << configuration.separator;
                }
            }
            *configuration.stream_ << "\n";
            isFirstLine = false;
        }
        for (auto &metric: metrics) {
            *configuration.stream_ << metric.second.value;
            if (&metric != &metrics.back()){
                    *configuration.stream_ << configuration.separator;
                }
        }
        *configuration.stream_ << "\n";
    }
};