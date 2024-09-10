#include <gflags/gflags.h>
#include <iostream>
#include <cstdint>
#include "helper_cuda.h"

#include "storage.hpp"
#include "util.hpp"
#include "dev_util.cuh"

#include "MonitoringInterface.h"

DEFINE_uint32(cuda_device, 0, "Index of CUDA device to use.");
DEFINE_uint32(scale_factor, 8, "Scale factor == size in GB.");
DEFINE_uint32(kernel_ms, 200, "Time to waste in GPU Kernel.");
DEFINE_uint32(store_offset, 0, "Starting offset in file or block device.");
DEFINE_string(ssdpath, "/raid/gds/300G.file", "Path to block device or file.");

int main(int argc, char *argv[]){
    start_monitoring();
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    checkCudaErrors(cudaSetDevice(FLAGS_cuda_device));
    StorageManager::get().init(FLAGS_ssdpath);
    const int THREAD_NUM = 8;

    { // stack for cuda-memcheck
    uint64_t bytes = FLAGS_scale_factor * uint64_t(1<<30);
    uint64_t block_size = bytes/THREAD_NUM;
    util::Log::get().info_fmt("Scale Factor %llu, %.2f GiB, Block Size %llu, %.2f GiB", bytes, bytes / double(1<<30),
                                block_size, block_size / double(1<<30));
    char *hst_ptr,*dev_ptr;
    cudaStream_t streams[THREAD_NUM];
    for (int i = 0; i<THREAD_NUM; ++i) checkCudaErrors(cudaStreamCreate(streams+i));

    checkCudaErrors(cudaMallocHost(&hst_ptr,bytes));
    checkCudaErrors(cudaMalloc(&dev_ptr,bytes));
    if (uint64_t(dev_ptr) % (1<<12) != 0){
        util::Log::get().info_fmt("dev ptr %p", dev_ptr);
    }
    if (uint64_t(hst_ptr) % (1<<12) != 0){
        util::Log::get().info_fmt("hst ptr %p", hst_ptr);
    }
    checkCuFileError(cuFileBufRegister(dev_ptr, bytes, 0));

    util::Timer timer;

    util::ThreadPool::parallel_n(THREAD_NUM, [&](int tid) {
        checkCudaErrors(cudaSetDevice(FLAGS_cuda_device));
        // 1) read block
        auto ret = cuFileRead(StorageManager::get().cfh, dev_ptr, block_size, FLAGS_store_offset + tid*block_size, tid*block_size);
        if (ret != block_size) util::Log::get().info_fmt("Tried reading %llu bytes, but read %llu", block_size, ret);
        // 2) Waste some time on GPU
        util::waiting_kernel<<<1,1,0,streams[tid]>>>(FLAGS_kernel_ms * 1000);
        // 3) Copy to main memory
        checkCudaErrors(cudaMemcpyAsync(hst_ptr, dev_ptr, bytes, cudaMemcpyDefault, streams[tid]));
        // 4) Also waste some CPU and Mem resources
        // Note: cudaMemcpyHostToHost usually not what you want to do
        checkCudaErrors(cudaMemcpyAsync(hst_ptr + tid*block_size, hst_ptr + tid*block_size + (block_size>>1), (block_size>>1), cudaMemcpyDefault, streams[tid]));
        // 5) Write to storage
        checkCudaErrors(cudaStreamSynchronize(streams[tid]));
        StorageManager::get().host_write_bytes(hst_ptr + tid*block_size, block_size, FLAGS_store_offset + tid*block_size);
    });

    stop_monitoring();
    util::Log::get().info_fmt("Total took %.2f ms", timer.elapsed());

    // clean up
    for (int i = 0; i<THREAD_NUM; ++i) checkCudaErrors(cudaStreamDestroy(streams[i]));
    checkCuFileError(cuFileBufDeregister(dev_ptr));
   
    checkCudaErrors(cudaFreeHost(hst_ptr));
    checkCudaErrors(cudaFree(dev_ptr));
    } // stack for cuda-memcheck
    checkCudaErrors(cudaDeviceReset());
}
