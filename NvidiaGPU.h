#include "Device.hpp"
#include "nvml.h"
#include "vector"
#include <chrono>

class NvidiaGPU final: public Device{
    bool first = true;
        std::chrono::time_point<std::chrono::steady_clock> startTime;
        std::chrono::time_point<std::chrono::steady_clock> stopTime;

    public:
        NvidiaGPU() : NvidiaGPU(getAllowedMetrics()){}
        explicit NvidiaGPU(const std::vector<Metric>& metrics);
        ~NvidiaGPU() override;

        std::vector<std::pair<Metric, Measurement>> getData(Sampler sampler) override;
        Measurement fetchMetric(const Metric &metric) override;

        void start();
        void stop();
};
