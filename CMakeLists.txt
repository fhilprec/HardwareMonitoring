# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.25)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
        HardwareMonitoring
        VERSION 0.1
        LANGUAGES CXX)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

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
add_library(HardwareMonitoring Device.cpp Device.hpp Counter.cpp Counter.hpp
        Output.hpp
        Output.cpp
        OutputConfiguration.cpp
        OutputConfiguration.hpp
        SamplingMethod.cpp
        SamplingMethod.h
        Metric.cpp
        Metric.h
        Measurement.cpp
        Measurement.h
        CPUPerf.cpp
        CPUPerf.h
        Calculator.cpp
        Calculator.h
        Monitor.cpp
        Monitor.h
        FileManager.cpp
        FileManager.h
        DependencyChecker.cpp
        DependencyChecker.h
        IOFile.cpp
        Graph.cpp
        Graph.h
)

# Link each target with other targets or add options, etc.
target_link_libraries(HardwareMonitoring fmt::fmt)


# Adding something we can run - Output name matches target name
add_executable(TestHardwareMonitoring test.cpp)

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
target_link_libraries(TestHardwareMonitoring HardwareMonitoring)
