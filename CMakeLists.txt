cmake_minimum_required(VERSION 3.0.0)
project(HardwareMonitoring VERSION 0.1.0 LANGUAGES C CXX)

add_library(HardwareMonitoring src/HardwareMonitoring.cpp)
target_include_directories(HardwareMonitoring PUBLIC "includes/")

