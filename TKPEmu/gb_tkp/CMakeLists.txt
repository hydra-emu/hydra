cmake_minimum_required(VERSION 3.19)
project(gb_tkp)
set(GB_TKP_REMOTE_ROOT https://github.com/OFFTKP/TKPEmu.git)
set(GB_TKP_REMOTE_DIR https://raw.githubusercontent.com/OFFTKP/GameboyTKP/master)
set(GB_TKP_FILES gameboy.h gameboy.cpp gb_cpu.h gb_cpu.cpp gb_ppu.h gb_ppu.cpp 
gb_addresses.h gb_bus.h gb_bus.cpp gb_timer.h gb_timer.cpp
gb_cartridge.h gb_cartridge.cpp)
DOWNLOAD_FILES(${GB_TKP_REMOTE_ROOT} ${GB_TKP_REMOTE_DIR} ${GB_TKP_FILES})
file(GLOB SRC_FILES "*.cpp")
file(GLOB HDR_FILES "*.h")
add_library(gb_tkp ${HDR_FILES} ${SRC_FILES})