#include <vector>
#include <string>
#include "OutputConfiguration.hpp"
#include "Device.hpp"

class Output{
private:
    OutputConfiguration *configuration;
    bool isFirstLine = true;

public:
    explicit Output(OutputConfiguration &configuration){
        this->configuration = configuration;
    }
    void writeLine(std::vector<std::pair<Metric, double>> metrics){
        if(isFirstLine || !configuration->fileMode){
            for(size_t i = 0; i < metrics.size(); ++i){
                configuration->stream_ << metrics[i]->first->name << configuration->seperator;
            }
            if(i == metrics.size()-1){
                configuration->stream_ << configuration->seperator;
            }
            isFirstLine = false;
        } else {
            for(size_t i = 0; i < metrics.size(); ++i){
                configuration->stream_ << std::to_string(metrics[i]->second);
                if(i == metrics.size()-1){
                    configuration->stream_ << configuration->seperator;
                }
            }
            configuration->stream_ << std::endl;
        }
    }

};