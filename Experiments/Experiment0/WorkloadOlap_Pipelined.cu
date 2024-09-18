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
#define NUM_STREAMS 4

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

void performGPUWork(float *d_A, float *d_B, float *d_C, int n, cudaStream_t stream) {
    dim3 threadsPerBlock(32, 32);
    dim3 blocksPerGrid((MATRIX_SIZE + threadsPerBlock.x - 1) / threadsPerBlock.x,
                       (MATRIX_SIZE + threadsPerBlock.y - 1) / threadsPerBlock.y);

    for (int i = 0; i < n; ++i) {
        matrixMultiply<<<blocksPerGrid, threadsPerBlock, 0, stream>>>(d_A, d_B, d_C, MATRIX_SIZE);
    }
}

void performCPUWork(int tid) {
    util::Timer::sleep_ms(200);
    int sum = 0;
    for(int i = 0; i < 10000000; i++){
        sum += i * i * i * i;
    }
}


int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    checkCudaErrors(cudaSetDevice(FLAGS_cuda_device));
    StorageManager::get().init(FLAGS_ssdpath);

    uint64_t bytes = FLAGS_scale_factor * uint64_t(1<<30);
    uint64_t chunk_size = bytes / NUM_STREAMS;
    util::Log::get().info_fmt("Scale Factor %llu, %.2f GiB, Chunk Size %llu, %.2f GiB",
                              bytes, bytes / double(1<<30),
                              chunk_size, chunk_size / double(1<<30));

    std::vector<char*> hst_ptrs(NUM_STREAMS);
    std::vector<char*> dev_ptrs(NUM_STREAMS);
    std::vector<cudaStream_t> streams(NUM_STREAMS);
    std::vector<float*> d_A(NUM_STREAMS), d_B(NUM_STREAMS), d_C(NUM_STREAMS);

    for (int i = 0; i < NUM_STREAMS; ++i) {
        checkCudaErrors(cudaMallocHost(&hst_ptrs[i], chunk_size));
        checkCudaErrors(cudaMalloc(&dev_ptrs[i], chunk_size));
        checkCudaErrors(cudaStreamCreate(&streams[i]));
        checkCuFileError(cuFileBufRegister(dev_ptrs[i], chunk_size, 0));

        // Allocate memory for matrix multiplication
        checkCudaErrors(cudaMalloc(&d_A[i], MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
        checkCudaErrors(cudaMalloc(&d_B[i], MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
        checkCudaErrors(cudaMalloc(&d_C[i], MATRIX_SIZE * MATRIX_SIZE * sizeof(float)));
    }

    util::Timer total_timer;
    start_monitoring();

    for (int repeat = 0; repeat < FLAGS_per_op_repeat; ++repeat) {
        for (int i = 0; i < NUM_STREAMS; ++i) {
            uint64_t offset = FLAGS_store_offset + i * chunk_size;
            
            // 1. Read from storage (asynchronous)
            auto ret = cuFileRead(StorageManager::get().cfh, dev_ptrs[i], chunk_size, offset, 0);
            if (ret != chunk_size) util::Log::get().info_fmt("Tried reading %llu bytes, but read %llu", chunk_size, ret);

            // 2. Perform GPU work (asynchronous)
            performGPUWork(d_A[i], d_B[i], d_C[i], 5, streams[i]);

            // 3. Copy to main memory (asynchronous)
            checkCudaErrors(cudaMemcpyAsync(hst_ptrs[i], dev_ptrs[i], chunk_size, cudaMemcpyDeviceToHost, streams[i]));

            // 4. Perform CPU work (asynchronous)
            std::thread cpu_thread(performCPUWork, i);
            cpu_thread.detach();  // Allow the thread to run independently

            // 5. Write to storage (will be synchronized before the next iteration)
            cudaStreamSynchronize(streams[i]);  // Ensure all operations are complete before writing
            StorageManager::get().host_write_bytes(hst_ptrs[i], chunk_size, offset);
        }
    }

    // Wait for all streams to complete
    for (int i = 0; i < NUM_STREAMS; ++i) {
        cudaStreamSynchronize(streams[i]);
    }

    util::Log::get().info_fmt("Total took %.2f ms", total_timer.elapsed());

    stop_monitoring();

    // Clean up
    for (int i = 0; i < NUM_STREAMS; ++i) {
        checkCuFileError(cuFileBufDeregister(dev_ptrs[i]));
        checkCudaErrors(cudaFreeHost(hst_ptrs[i]));
        checkCudaErrors(cudaFree(dev_ptrs[i]));
        checkCudaErrors(cudaFree(d_A[i]));
        checkCudaErrors(cudaFree(d_B[i]));
        checkCudaErrors(cudaFree(d_C[i]));
        checkCudaErrors(cudaStreamDestroy(streams[i]));
    }

    checkCudaErrors(cudaDeviceReset());
    return 0;
}