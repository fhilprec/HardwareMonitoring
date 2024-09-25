#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include "Device.hpp"
#include "CPUPerf.h"
#include "IOFile.h"
#include "Monitor.h"
#include "GPUFile.h"

// Function to perform some CPU-intensive work
void doCPUWorkFlops(unsigned int numIterations)
{
    double a = 124.23525, b = 21.2412, c = 2342.23432, d = 23.324, e = 2.3412, f = 123.21, g = 1231.12, h = 567.4, j =
               34.4, k = 24, l = 342.24,
           m = 324.23525, n = 51.2412, o = 242.23432, p = 254.324, q = 112.3412, r = 853.21, s = 31.12, t = 67.4, u =
               34.4, v = 4, w = 3.24, x = 89.131, y = 123.23, z = 123.123,
           v1 = 12.3, v2 = 3, v3 = 56.5, v4 = 88.56, v5 = 787.43, v6 = 0, v7 = 0, v8 = 0, v9 = 0, v10 = 1, v11 = 54, v12
               = 12, v13 = 45, v14 = 5.66, v15 = 123.12, v16 = 1,
           v17 = 1, v18 = 12, v19 = 12, v20 = 12.1, v21 = 1.1, v22 = 12;

    for (int i2=0;i2<numIterations;i2++)
    {
        for (int i = 0; i < 10000000;i++)
        {
            a = a + b;
            c = d + e;
            f = g * h;
            m = m + n;
            j = k * l;


            q = o + p;
            s = r + s;
            v = t * u;
            m = k + n;
            j = w * l;

            x = x * y;
            v6 = v1 + v2;
            v7 = v3 * v4;
            v8 = v5 * v5;
            v9 = g * k;
        }
    }
}

static int pin_thread(int core_id, int pid = 0){
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    return sched_setaffinity(pid, sizeof(cpu_set_t), &cpuset);
}


void DoWork(unsigned int numIterations, unsigned int numThreads)
{
    std::vector<std::jthread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back([=]
        {
            pin_thread(i%16);
            doCPUWorkFlops(1);
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}

// monitor an intense cpu task
int main(int argc, char* argv[])
{
    bool useMonitoring;
    unsigned int numIterations;
    unsigned int numThreads;
    if(argc == 4)
    {
        useMonitoring = "y" == std::string(argv[1]);
        numIterations = std::stoul(argv[2]);
        numThreads = std::stoul(argv[3]);
    }else
    {
        std::cerr << "Use this programm with params:\n1. useMonitoring [y|n]\n2. num Iterations [number]\n3.num Threads [number]";
        return 0;
    }
    //config
    std::string outputFolder = "test_intense_cpu_3/temp_output";

    //test
    std::vector<std::shared_ptr<IDevice>> devices;

    auto* device2 = new CPUPerf();
    devices.emplace_back((IDevice*)device2);

    std::filesystem::path outputDirectory(outputFolder);
    auto fullPath = absolute(outputDirectory);

    if(useMonitoring)
    {
        Monitor monitor({devices, std::chrono::milliseconds(100), outputDirectory, outputDirectory});

        monitor.start();
        DoWork(numIterations, numThreads);
        monitor.stop();
    }else
    {
        DoWork(numIterations, numThreads);
    }

    return 0;
}
