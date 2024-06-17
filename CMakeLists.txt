cmake_minimum_required(VERSION 3.28)
project(HardwareMonitoring)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(HardwareMonitoring
        Counter.hpp
        Device.hpp
        PerfEvent.hpp
        test.cpp)
