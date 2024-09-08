# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.25)

# This is your project statement. You should always list languages;


# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++ -lm")


set(CMAKE_C_COMPILER "gcc-14.2.0")
set(CMAKE_CXX_COMPILER "/home/fhilprecht/gcc-14.2.0/bin/gcc-11.1")
# Listing the version is nice here since it sets lots of useful variables
project(
        HardwareMonitoring
        VERSION 0.1
        LANGUAGES CXX)


find_package(Threads REQUIRED)


# Find packages go here.
include(cmake/CPM.cmake)
CPMAddPackage(
        NAME fmt
        GIT_TAG 11.0.2
        GITHUB_REPOSITORY fmtlib/fmt # to get an installable target
        OPTIONS "FMT_INSTALL YES"
)
# You should usually split this into folders, but this is a simple example

# This is a "default" library, and will match the *** variable setting.
# Other common choices are STATIC, SHARED, and MODULE
# Including header files here helps IDEs but is not required.
# Output libname matches target name, with the usual extensions on your system
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
        Devices/IOFile.cpp
        Devices/IOFile.h
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
)


target_include_directories(HardwareMonitoring PUBLIC
    ${CMAKE_SOURCE_DIR}/Devices
    ${CMAKE_SOURCE_DIR}/Core
)


# Link each target with other targets or add options, etc.
target_link_libraries(HardwareMonitoring fmt::fmt)


# Adding something we can run - Output name matches target name
add_executable(TestHardwareMonitoring Experiments/Experiment0/WorkloadRunEverything.cpp)

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
target_link_libraries(TestHardwareMonitoring HardwareMonitoring)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
