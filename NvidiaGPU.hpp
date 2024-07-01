#include "Device.hpp"
#include "nvml.h"
#include "vector"
#include <chrono>

class NvidiaGPU final: public Device{
    nvmlReturn_t result;
    nvmlDevice_t device;

    const unsigned int GPUNumber = 0;


    nvmlMemory_t getMemoryInformation();

public:
    NvidiaGPU() : NvidiaGPU(getAllowedMetrics(), GPUNumber){}
    explicit NvidiaGPU(const std::vector<Metric>& metrics, const unsigned int GPUNumber);

    Measurement fetchMetric(const Metric &metric) override;

};
