cmake_minimum_required(VERSION 3.25)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++ -lm")

set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --expt-relaxed-constexpr")

# Set CUDA standard to 14 (highest supported by most CUDA versions)
set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_CUDA_STANDARD_REQUIRED YES)
set(DCUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda-11.8")
set(CMAKE_C_COMPILER "gcc-14.2.0")
set(CMAKE_CXX_COMPILER "/home/fhilprecht/gcc-14.2.0/bin/gcc-11.1")

project(
        HardwareMonitoring
        VERSION 0.1
        LANGUAGES CXX CUDA)

find_package(Threads REQUIRED)
find_package(CUDA REQUIRED)
find_package(gflags REQUIRED)

# Find CUDA-specific libraries
find_library(CUDA_DRIVER_LIBRARY cuda)
find_library(CUDA_NVRTC_LIBRARY nvrtc)

include(cmake/CPM.cmake)
CPMAddPackage(
        NAME fmt
        GIT_TAG 11.0.2
        GITHUB_REPOSITORY fmtlib/fmt
        OPTIONS "FMT_INSTALL YES"
)

add_library(HardwareMonitoring 
        Devices/Device.cpp 
        Devices/Device.hpp 
        Core/Counter.cpp 
        Core/Counter.hpp
        Core/Output.hpp
        Core/Output.cpp
        Core/OutputConfiguration.cpp
        Core/OutputConfiguration.hpp
        Core/SamplingMethod.cpp
        Core/SamplingMethod.h
        Core/Metric.cpp
        Core/Metric.h
        Core/Measurement.cpp
        Core/Measurement.h
        Devices/Polling/CPUPerf.cpp
        Devices/Polling/CPUPerf.h
        Core/Calculator.cpp
        Core/Calculator.h
        Core/Monitor.cpp
        Core/Monitor.h
        Core/FileManager.cpp
        Core/FileManager.h
        Core/DependencyChecker.cpp
        Core/DependencyChecker.h
        Devices/Polling/IOFile.cpp
        Devices/Polling/IOFile.h
        Core/Graph.cpp
        Core/Graph.h
        Devices/Polling/GPUFile.cpp
        Devices/Polling/GPUFile.h
        Devices/Polling/NIC.cpp
        Devices/Polling/NIC.h
        Devices/TwoShot/CPUPerfTwoShot.cpp
        Devices/TwoShot/CPUPerfTwoShot.h
        Devices/TwoShot/IOFileTwoShot.cpp
        Devices/TwoShot/IOFileTwoShot.h
        Experiments/Experiment0/MonitoringInterface.cpp
)

target_include_directories(HardwareMonitoring PUBLIC
    ${CMAKE_SOURCE_DIR}/Devices
    ${CMAKE_SOURCE_DIR}/Devices/Polling
    ${CMAKE_SOURCE_DIR}/Devices/TwoShot
    ${CMAKE_SOURCE_DIR}/Core
    ${CMAKE_SOURCE_DIR}/Experiments/Experiment3
    ${CMAKE_SOURCE_DIR}/Experiments/Experiment0/CudaHelperFiles
    ${CUDA_INCLUDE_DIRS}
    ${gflags_INCLUDE_DIRS}
)

target_link_libraries(HardwareMonitoring 
    fmt::fmt
    ${CUDA_LIBRARIES}
    ${CUDA_DRIVER_LIBRARY}
    ${CUDA_NVRTC_LIBRARY}
    ${gflags_LIBRARIES}
    cuda
    cudart
)

add_executable(WorkloadRunEverything Experiments/Experiment0/WorkloadRunEverything.cpp)
add_executable(Experiment3_IOFile_Raw Experiments/Experiment3/IOFile_Raw.cpp)
add_executable(Experiment3_IOFile_Monitored  Experiments/Experiment3/IOFile_Monitored.cpp)
add_executable(Experiment3_Perf_Raw  Experiments/Experiment3/Perf_Raw.cpp)
add_executable(Experiment3_Perf_Monitored  Experiments/Experiment3/Perf_Monitored.cpp)

target_link_libraries(WorkloadRunEverything HardwareMonitoring)
target_link_libraries(Experiment3_IOFile_Raw HardwareMonitoring)
target_link_libraries(Experiment3_IOFile_Monitored HardwareMonitoring)
target_link_libraries(Experiment3_Perf_Raw HardwareMonitoring)
target_link_libraries(Experiment3_Perf_Monitored HardwareMonitoring)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

# CUDA-specific part
add_executable(WorkloadOlap_Blocking Experiments/Experiment0/WorkloadOlap_Blocking.cu)
add_executable(WorkloadOlap_Pipelined Experiments/Experiment0/WorkloadOlap_Pipelined.cu)

set_target_properties(WorkloadOlap_Blocking PROPERTIES
    CUDA_ARCHITECTURES "60;70;75;80"
)

target_link_libraries(WorkloadOlap_Blocking 
    HardwareMonitoring
    ${CUDA_LIBRARIES}
    ${CUDA_DRIVER_LIBRARY}
    ${CUDA_NVRTC_LIBRARY}
    ${gflags_LIBRARIES}
    cuda
    cudart
    cufile
)

target_link_libraries(WorkloadOlap_Pipelined
    HardwareMonitoring
    ${CUDA_LIBRARIES}
    ${CUDA_DRIVER_LIBRARY}
    ${CUDA_NVRTC_LIBRARY}
    ${gflags_LIBRARIES}
    cuda
    cudart
    cufile

)
if(UNIX)
    target_link_libraries(WorkloadOlap_Blocking -lcudart)
endif()

if(UNIX)
    target_link_libraries(WorkloadOlap_Pipelined -lcudart)
endif()