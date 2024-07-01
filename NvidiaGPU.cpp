#include <vector>
#include "nvml.h"
#include "NvidiaGPU.hpp"

nvmlReturn_t result;
nvmlDevice_t device;
const unsigned int GPUNumber;

NvidiaGPU::NvidiaGPU(const std::vector<Metric>& metrics, const unsigned int GPUNumber): Device("NvidiaGPU",metrics){
    
    std::vector<Metric> pollingMetrics = {Metric(TWO_SHOT, "used_memory")};

    result = nvmlInit_v2();
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << std::endl;
    }

    result = nvmlDeviceGetHandleByIndex_v2(GPUNumber, &device);
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to get handle for device " << GPUNumber << ": " << nvmlErrorString(result) << std::endl;
    }

};

nvmlMemory_t NvidiaGPU::getMemoryInformation(){
    nvmlMemory_t memory;
    result = nvmlDeviceGetMemoryInfo(device, &memory);
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to get memory info for device " << GPUNumber << ": " << nvmlErrorString(result) << std::endl;
    }
    return memory;
};

Measurement NvidiaGPU::fetchMetric(const Metric& metric){

    if(metric.name=="used_memory"){
        return Measurement(std::to_string(getMemoryInformation().used));
    }

};