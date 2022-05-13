cmake_minimum_required(VERSION 3.19)
project(TKPSrc)
set(FILES display.cpp emulator_factory.cpp emulator_results.cpp
    emulator.cpp settings_manager.cpp base_application.cpp)
add_library(TKPSrc ${FILES})