cmake_minimum_required(VERSION 3.19)
project(TKPSrc)
set(FILES emulator.cpp emulator_factory.cpp emulator_results.cpp
    settings_manager.cpp)
add_library(TKPSrc ${FILES})
target_include_directories(TKPSrc PUBLIC ../)