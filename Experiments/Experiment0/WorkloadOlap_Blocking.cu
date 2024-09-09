#include <gflags/gflags.h>
#include <iostream>
#include <cstdint>
#include "helper_cuda.h"

#include "storage.hpp"
#include "util.hpp"
#include "dev_util.cuh"


DEFINE_uint32(cuda_device, 0, "Index of CUDA device to use.");
DEFINE_uint32(scale_factor, 5, "Scale factor == size in GB.");
DEFINE_uint32(per_op_repeat, 3, "Repetition of each operation.");
DEFINE_uint32(kernel_ms, 200, "Time to waste in GPU Kernel.");
DEFINE_uint32(store_offset, 0, "Starting offset in file or block device.");
DEFINE_string(ssdpath, "/raid/gds/300G.file", "Path to block device or file.");

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

    // 2) Waste some time on GPU
    for (int i = 0; i < FLAGS_per_op_repeat; ++i){
        util::waiting_kernel<<<1,1>>>(FLAGS_kernel_ms * 1000);
        checkCudaErrors(cudaDeviceSynchronize());
    }
    util::Log::get().info_fmt("Kernels took %.2f ms", timer.elapsed());
    timer.reset();

    // 3) Copy to main memory
    for (int i = 0; i < FLAGS_per_op_repeat; ++i){
        checkCudaErrors(cudaMemcpy(hst_ptr, dev_ptr, bytes, cudaMemcpyDefault));
    }
    util::Log::get().info_fmt("Memcpys took %.2f ms", timer.elapsed());
    timer.reset();

    // 4) Also waste some time on CPU
    for (int i = 0; i < FLAGS_per_op_repeat; ++i){
        util::ThreadPool::parallel_n(8, [&](int tid) {
            util::Timer::sleep_ms(200);
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


    // clean up
    checkCuFileError(cuFileBufDeregister(dev_ptr));
    checkCudaErrors(cudaFreeHost(hst_ptr));
    checkCudaErrors(cudaFree(dev_ptr));
    } // stack for cuda-memcheck


    checkCudaErrors(cudaDeviceReset());
}