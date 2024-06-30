#include "NvidiaGPU.h"
#include <vector>

NvidiaGPU::NvidiaGPU(const std::vector<Metric>& metrics): Device("CPUPerf",metrics){
    
    std::vector<Metric> pollingMetrics = {Metric(TWO_SHOT, "cycles"),
        Metric(TWO_SHOT, "kcycles"),
        Metric(TWO_SHOT, "instructions"),
        Metric(TWO_SHOT, "L1-misses"),
        Metric(TWO_SHOT, "LLC-misses"),
        Metric(TWO_SHOT, "branch-misses")};

};