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


// Function to simulate GPU work (vector addition)
void simulateGPUWork(const std::vector<float>& A, const std::vector<float>& B, std::vector<float>& C)
{
    for (size_t i = 0; i < A.size(); ++i)
    {
        C[i] = A[i] + B[i];
    }
}

// Function to perform some simulated GPU work
void doSimulatedGPUWork(int numElements)
{
    std::vector<float> h_A(numElements);
    std::vector<float> h_B(numElements);
    std::vector<float> h_C(numElements);

    // Initialize input vectors
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    for (int i = 0; i < numElements; ++i)
    {
        h_A[i] = dis(gen);
        h_B[i] = dis(gen);

        if (i % (numElements / 5) == 0)
        {
            int ret = system("/usr/local/cuda-12/gds/tools/gdsio -d 0 -w 4 -s 4G -i 1M -x 0 -I 0 -f /dev/md127");
            // Check if the command executed successfully
            if (ret != 0)
            {
                std::cerr << "Command execution failed at element " << i << std::endl;
                break;
            }
        }
    }

    // Perform the simulated GPU work
    simulateGPUWork(h_A, h_B, h_C);

    // Verify the result (optional)
    for (int i = 0; i < numElements; ++i)
    {
        if (std::abs(h_A[i] + h_B[i] - h_C[i]) > 1e-5)
        {
            std::cerr << "Result verification failed at element " << i << std::endl;
            break;
        }
    }
}


// Function to perform some CPU-intensive work
void doCPUWorkFlops()
{
    double a = 124.23525, b = 21.2412, c = 2342.23432, d = 23.324, e = 2.3412, f = 123.21, g = 1231.12, h = 567.4, j =
               34.4, k = 24, l = 342.24,
           m = 324.23525, n = 51.2412, o = 242.23432, p = 254.324, q = 112.3412, r = 853.21, s = 31.12, t = 67.4, u =
               34.4, v = 4, w = 3.24, x = 89.131, y = 123.23, z = 123.123,
           v1 = 12.3, v2 = 3, v3 = 56.5, v4 = 88.56, v5 = 787.43, v6 = 0, v7 = 0, v8 = 0, v9 = 0, v10 = 1, v11 = 54, v12
               = 12, v13 = 45, v14 = 5.66, v15 = 123.12, v16 = 1,
           v17 = 1, v18 = 12, v19 = 12, v20 = 12.1, v21 = 1.1, v22 = 12;

    for (int j=0;j<100000;j++)
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

void DoWork()
{
    std::vector<std::jthread> threads;
    threads.reserve(std::thread::hardware_concurrency() - 1);
    for (int i = 0; i < std::thread::hardware_concurrency() - 1; i++)
    {
        threads.emplace_back(&doCPUWorkFlops);
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
    if(argc == 2)
    {
        useMonitoring = "useMonitoring" == std::string(argv[1]);
    }else
    {
        useMonitoring = false;
    }
    //config
    std::string outputFolder = "test_intense_cpu_1";

    //test
    std::vector<std::shared_ptr<IDevice>> devices;

    auto* device2 = new CPUPerf();
    devices.emplace_back((IDevice*)device2);

    std::filesystem::path outputDirectory(outputFolder);
    auto fullPath = absolute(outputDirectory);

    if(useMonitoring)
    {
        Monitor monitor({devices, std::chrono::milliseconds(500), outputDirectory, outputDirectory});

        monitor.start();
        DoWork();
        monitor.stop();
    }else
    {
        DoWork();
    }

    return 0;
}
