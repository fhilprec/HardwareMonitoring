#include <gflags/gflags.h>
#include <iostream>
#include <cstdint>
#include "helper_cuda.h"

#include "storage.hpp"
#include "util.hpp"
#include "dev_util.cuh"

#include "MonitoringInterface.h"


DEFINE_uint32(cuda_device, 0, "Index of CUDA device to use.");
DEFINE_uint32(scale_factor, 5, "Scale factor == size in GB.");
DEFINE_uint32(per_op_repeat, 3, "Repetition of each operation.");
DEFINE_uint32(kernel_ms, 200, "Time to waste in GPU Kernel.");
DEFINE_uint32(store_offset, 0, "Starting offset in file or block device.");
DEFINE_string(ssdpath, "/raid/gds/300G.file", "Path to block device or file.");



#include <cuda_runtime.h>

// Define matrix dimensions
#define MATRIX_SIZE 4096

// CUDA kernel for matrix multiplication
__global__ void matrixMultiply(float *A, float *B, float *C, int width) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < width && col < width) {
        float sum = 0.0f;
        for (int i = 0; i < width; ++i) {
            sum += A[row * width + i] * B[i * width + col];
        }
        C[row * width + col] = sum;
    }
}

// Function to initialize a matrix with random values
void initializeMatrix(float *matrix, int size) {
    for (int i = 0; i < size * size; ++i) {
        matrix[i] = static_cast<float>(rand()) / RAND_MAX;
    }
}

// Replace step 2 with this function
void performGPUWork(int n, uint32_t kernel_ms) {
    const int matrixSize = MATRIX_SIZE * MATRIX_SIZE;
    const int matrixBytes = matrixSize * sizeof(float);

    float *h_A, *h_B, *h_C;
    float *d_A, *d_B, *d_C;

    // Allocate host memory
    h_A = new float[matrixSize];
    h_B = new float[matrixSize];
    h_C = new float[matrixSize];

    // Initialize matrices
    initializeMatrix(h_A, MATRIX_SIZE);
    initializeMatrix(h_B, MATRIX_SIZE);

    // Allocate device memory
    cudaMalloc(&d_A, matrixBytes);
    cudaMalloc(&d_B, matrixBytes);
    cudaMalloc(&d_C, matrixBytes);

    // Copy data to device
    cudaMemcpy(d_A, h_A, matrixBytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, matrixBytes, cudaMemcpyHostToDevice);

    // Setup execution parameters
    dim3 threadsPerBlock(32, 32);
    dim3 blocksPerGrid((MATRIX_SIZE + threadsPerBlock.x - 1) / threadsPerBlock.x,
                       (MATRIX_SIZE + threadsPerBlock.y - 1) / threadsPerBlock.y);

    // Perform matrix multiplication multiple times
    for (int i = 0; i < n; ++i) {
        matrixMultiply<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C, MATRIX_SIZE);
        cudaDeviceSynchronize();
    }

    // Copy result back to host
    cudaMemcpy(h_C, d_C, matrixBytes, cudaMemcpyDeviceToHost);

    // Clean up
    delete[] h_A;
    delete[] h_B;
    delete[] h_C;
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);
}

int main(int argc, char *argv[]){
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    checkCudaErrors(cudaSetDevice(FLAGS_cuda_device));
    StorageManager::get().init(FLAGS_ssdpath);

    { // stack for cuda-memcheck
    uint64_t bytes = FLAGS_scale_factor * uint64_t(1<<30);
    uint64_t block_size = bytes>>3;
    util::Log::get().info_fmt("Scale Factor %llu, %.2f GiB, Block Size %llu, %.2f GiB",
                                bytes, bytes / double(1<<30),
                                block_size, block_size / double(1<<30));
    char *hst_ptr,*dev_ptr;
    checkCudaErrors(cudaMallocHost(&hst_ptr,bytes));
    checkCudaErrors(cudaMalloc(&dev_ptr,bytes));
    if (uint64_t(dev_ptr) % 4096 != 0){
        util::Log::get().info_fmt("dev ptr %p", dev_ptr);
    }
    if (uint64_t(hst_ptr) % 4096 != 0){
        util::Log::get().info_fmt("hst ptr %p", hst_ptr);
    }
    checkCuFileError(cuFileBufRegister(dev_ptr, bytes, 0));

    util::Timer total_timer;
    util::Timer timer;
    start_monitoring();
    // 1) Read from storage
    for (int i = 0; i < FLAGS_per_op_repeat; ++i){
        util::ThreadPool::parallel_n(8, [&](int tid) {
            // every thread one read for now
            auto ret = cuFileRead(StorageManager::get().cfh, dev_ptr, block_size, FLAGS_store_offset + tid*block_size,
                                    tid*block_size);
            if (ret != block_size) util::Log::get().info_fmt("Tried reading %llu bytes, but read %llu", block_size, ret);
        });
    }
    util::Log::get().info_fmt("Storage reads took %.2f ms", timer.elapsed());
    timer.reset();
    int n = 5;

    // 2) Waste some time on GPU
    for (int i = 0; i < n; ++i){
        // util::waiting_kernel<<<1,1>>>(FLAGS_kernel_ms * 0);
        performGPUWork(n, FLAGS_kernel_ms * 1);
        checkCudaErrors(cudaDeviceSynchronize());
    }
    util::Log::get().info_fmt("Kernels took %.2f ms", timer.elapsed());
    timer.reset();
        // std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    // 3) Copy to main memory
    for (int i = 0; i < n; ++i){
        checkCudaErrors(cudaMemcpy(hst_ptr, dev_ptr, bytes, cudaMemcpyDefault));
    }
    util::Log::get().info_fmt("Memcpys took %.2f ms", timer.elapsed());
    timer.reset();

    // 4) Also waste some time on CPU
    for (int i = 0; i < n; ++i){
        util::ThreadPool::parallel_n(8, [&](int tid) {
            util::Timer::sleep_ms(200);
            int sum = 0;
            for(int i  = 0; i < 10000000; i++){
                sum += i * i * i * i;
            }
        });
    }
    util::Log::get().info_fmt("CPU threads took %.2f ms", timer.elapsed());
    timer.reset();

    // 5) Write to storage
    for (int i = 0; i < FLAGS_per_op_repeat; ++i){
        StorageManager::get().host_write_bytes(hst_ptr, bytes, FLAGS_store_offset);
    }
    util::Log::get().info_fmt("Storage writes took %.2f ms", timer.elapsed());
    util::Log::get().info_fmt("Total took %.2f ms", total_timer.elapsed());

    stop_monitoring();
    // clean up
    checkCuFileError(cuFileBufDeregister(dev_ptr));
    checkCudaErrors(cudaFreeHost(hst_ptr));
    checkCudaErrors(cudaFree(dev_ptr));
    } // stack for cuda-memcheck


    checkCudaErrors(cudaDeviceReset());
}